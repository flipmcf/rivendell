// batch.cpp
//
// Batch Routines for the Rivendell netcatcher daemon
//
//   (C) Copyright 2002-2023 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <math.h>
#include <netdb.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <errno.h>

#include <QApplication>

#include <rdapplication.h>
#include <rdaudioconvert.h>
#include <rdconf.h>
#include <rdcut.h>
#include <rddb.h>
#include <rddatedecode.h>
#include <rddebug.h>
#include <rddownload.h>
#include <rdescape_string.h>
#include <rdfeed.h>
#include <rdpodcast.h>
#include <rdsettings.h>
#include <rdtempdirectory.h>
#include <rdupload.h>
#include <rdurl.h>
#include <rdwavefile.h>
#include <rdweb.h>

#include "rdcatchd.h"

void MainObject::userChangedData()
{
  QString err_msg;

  disconnect(rda,SIGNAL(userChanged()),this,SLOT(userChangedData()));

  //
  // Create RDCatchConf
  //
  catch_conf=new RDCatchConf(rda->config()->stationName());

  //
  // Dispatch Handler
  //
  switch(batch_event->type()) {
  case RDRecording::Recording:
    RunImport(batch_event);
    SendNotification(RDNotification::CartType,RDNotification::ModifyAction,
		     RDCut::cartNumber(batch_event->cutName()));
    break;

  case RDRecording::Download:
    if(RunDownload(batch_event,&err_msg)) {
      WriteExitCode(batch_event,RDRecording::Ok,"OK");
      SendEventResponse(0,RDDeck::Idle,batch_event->id(),"");
      SendNotification(RDNotification::CartType,RDNotification::ModifyAction,
		       RDCut::cartNumber(batch_event->cutName()));
    }
    else {
      WriteExitCode(batch_event,RDRecording::ServerError,err_msg);
      SendEventResponse(0,RDDeck::Offline,batch_event->id(),"");
    }
    break;

  case RDRecording::Upload:
    if(RunUpload(batch_event,&err_msg)) {
      WriteExitCode(batch_event,RDRecording::Ok,"OK");
      SendEventResponse(0,RDDeck::Idle,batch_event->id(),"");
    }
    else {
      WriteExitCode(batch_event,RDRecording::ServerError,err_msg);
      SendEventResponse(0,RDDeck::Offline,batch_event->id(),"");
    }
    break;

  default:
    fprintf(stderr,"rdcatchd: nothing to do for this event type\n");
    exit(256);
  }

  QTimer *timer=new QTimer(this);
  timer->setSingleShot(true);
  connect(timer,SIGNAL(timeout()),this,SLOT(exitData()));
  timer->start(5000);  // So notifications have a chance to propagate
}


void MainObject::exitData()
{
  exit(0);
}


void MainObject::RunBatch(RDCmdSwitch *cmd)
{
  bool ok=false;
  int id=-1;

  //
  // Set Process Priority
  //
  struct sched_param sp;
  memset(&sp,0,sizeof(sp));
  if(sched_setscheduler(getpid(),SCHED_BATCH,&sp)!=0) {
    rda->syslog(LOG_DEBUG,"unable to set batch permissions, %s",
		strerror(errno));
  }

  //
  // Get ID
  //
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--event-id") {
      id=cmd->value(i).toInt(&ok);
      if((!ok)||(id<0)) {
	fprintf(stderr,"rdcatchd: invalid event-id\n");
	exit(256);
      }
    }
  }
  if(id<0) {
    fprintf(stderr,"rdcatchd: missing event-id\n");
    exit(256);
  }

  //
  // Calculate Temporary Directory
  //
  catch_temp_dir=RDTempDirectory::basePath();

  //
  // Load Event
  //
  QString sql=LoadEventSql()+QString::asprintf(" where `ID`=%d",id);
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    fprintf(stderr,"rdcatchd: id %d not found\n",id);
    exit(256);
  }
  batch_event=new CatchEvent(rda->station(),RDConfiguration());
  LoadEvent(q,batch_event,false);
  delete q;

  //
  // Connect to ripcd(8)
  //
  connect(rda,SIGNAL(userChanged()),this,SLOT(userChangedData()));
  rda->ripc()->
    connectHost("localhost",RIPCD_TCP_PORT,rda->config()->password());
}


void MainObject::RunImport(CatchEvent *evt)
{
  QString err_msg;

  evt->setTempName(GetTempRecordingName(evt->id()));
  evt->setDeleteTempFile(true);
  Import(evt,&err_msg);
}


bool MainObject::RunDownload(CatchEvent *evt,QString *err_msg)
{
  RDDownload::ErrorCode conv_err;

  //
  // Resolve Wildcards
  //
  RDStation *station=new RDStation(rda->config()->stationName());
  evt->resolveUrl(station->timeOffset());
  delete station;

  //
  // Execute Download
  //
  evt->setTempName(BuildTempName(evt,"download"));
  rda->syslog(LOG_INFO,"starting download of %s to %s, id=%d",
	      (const char *)evt->resolvedUrl().toUtf8(),
	      (const char *)evt->tempName().toUtf8(),
	      evt->id());
  RDDownload *conv=new RDDownload(rda->config(),this);
  
  conv->setSourceUrl(RDUrlEscape(evt->resolvedUrl()));
  conv->setDestinationFile(evt->tempName());
  QString url_username=evt->urlUsername();
  QString url_password=evt->urlPassword();
  QString id_filename=rda->station()->sshIdentityFile();
  if(url_username.isEmpty()&&
     (QUrl(evt->resolvedUrl()).scheme().toLower()=="ftp")) {
    url_username=RD_ANON_FTP_USERNAME;
    url_password=QString(RD_ANON_FTP_PASSWORD)+"-"+VERSION;
  }
  
  conv_err=conv->runDownload(url_username,url_password,id_filename,
			     evt->useSshIdentity(),err_msg,
			     rda->config()->logXloadDebugData());
  if(conv_err==RDDownload::ErrorOk) {
    rda->syslog(LOG_INFO,"finished download of %s to %s, id=%d",
		evt->tempName().toUtf8().constData(),
		evt->resolvedUrl().toUtf8().constData(),
		evt->id());
  }
  else {
    rda->syslog(LOG_WARNING,"download of %s returned an error: \"%s\", id=%d",
		evt->tempName().toUtf8().constData(),
		RDDownload::errorText(conv_err).toUtf8().constData(),
		evt->id());
    delete conv;
    unlink(evt->tempName().toUtf8());
    return false;
  }
  delete conv;
  
  //
  // Execute Import
  //
  bool imported=Import(evt,err_msg);
  rda->syslog(LOG_INFO,"deleting file %s, id=%d",
	      evt->tempName().toUtf8().constData(),evt->id());
  unlink(evt->tempName().toUtf8());
  return imported;
}


bool MainObject::RunUpload(CatchEvent *evt,QString *err_msg)
{
  RDUpload::ErrorCode conv_err;

  if(evt->feedId()<=0) {  // Standard upload
    //
    // Resolve Wildcards
    //
    RDStation *station=new RDStation(rda->config()->stationName());
    evt->resolveUrl(station->timeOffset());
    delete station;

    //
    // Execute Export
    //
    evt->setTempName(BuildTempName(evt,"upload"));
    evt->setDeleteTempFile(true);
    rda->syslog(LOG_INFO,"started export of cut %s to %s, id=%d",
		evt->cutName().toUtf8().constData(),
		evt->tempName().toUtf8().constData(),evt->id());
    if(!Export(evt,err_msg)) {
      rda->syslog(LOG_WARNING,"export of cut %s returned an error, id=%d",
		  evt->cutName().toUtf8().constData(),
		  evt->id());
      return false;
    }
    rda->syslog(LOG_INFO,"finished export of cut %s to %s, id=%d",
		evt->cutName().toUtf8().constData(),
		evt->tempName().toUtf8().constData(),evt->id());

    //
    // Execute Upload
    //
    rda->syslog(LOG_INFO,"starting upload of %s to %s, id=%d",
		evt->tempName().toUtf8().constData(),
		evt->resolvedUrl().toUtf8().constData(),
		evt->id());

    RDUpload *conv=new RDUpload(rda->config(),this);
    conv->setSourceFile(evt->tempName());
    conv->setDestinationUrl(evt->resolvedUrl());
    QString url_username=evt->urlUsername();
    QString url_password=evt->urlPassword();
    QString id_filename=rda->station()->sshIdentityFile();
    if(url_username.isEmpty()&&
       (QUrl(evt->resolvedUrl()).scheme().toLower()=="ftp")) {
      url_username=RD_ANON_FTP_USERNAME;
      url_password=QString(RD_ANON_FTP_PASSWORD)+"-"+VERSION;
    }


    conv_err=conv->runUpload(url_username,url_password,id_filename,
			     evt->useSshIdentity(),err_msg,
			     rda->config()->logXloadDebugData());
    rda->syslog(LOG_NOTICE,"CONV_ERR: %u",conv_err);  

    if(conv_err==RDUpload::ErrorOk) {
      rda->syslog(LOG_INFO,"finished upload of %s to %s, id=%d",
		  (const char *)evt->tempName().toUtf8(),
		  (const char *)evt->resolvedUrl().toUtf8(),
		  evt->id());
    }
    else {
      rda->syslog(LOG_WARNING,"upload of %s returned an error: \"%s\", id=%d",
		  (const char *)evt->tempName().toUtf8(),
		  (const char *)RDUpload::errorText(conv_err).toUtf8(),
		  evt->id());
      delete conv;
      return false;
    }
    delete conv;

    //
    // Clean Up
    //
    if(evt->deleteTempFile()) {
      unlink(evt->tempName().toUtf8());
      rda->
	syslog(LOG_INFO,"deleted file %s",evt->tempName().toUtf8().constData());
    }
    else {
      RDCheckExitCode("batch.cpp chown",chown(evt->tempName().toUtf8(),
					      rda->config()->uid(),
					      rda->config()->gid()));
    }
  }
  else {  // Podcast upload
    unsigned cast_id;
    RDFeed *feed=new RDFeed(evt->feedId(),rda->config(),this);
    rda->syslog(LOG_INFO,"starting post of %s to feed \"%s\", id=%d",
		evt->tempName().toUtf8().constData(),
		feed->keyName().toUtf8().constData(),
		evt->id());
    if((cast_id=feed->postCut(evt->cutName(),err_msg))==0) {
      rda->syslog(LOG_WARNING,"post of %s to feed \"%s\" failed [%s], id=%d",
		  evt->cutName().toUtf8().constData(),
		  feed->keyName().toUtf8().constData(),
		  err_msg->toUtf8().constData(),
		  evt->id());
      delete feed;
      return false;
    }
    rda->syslog(LOG_INFO,"post of %s to cast id %u successful, id=%d",
		evt->tempName().toUtf8().constData(),
		cast_id,
		evt->id());
    delete feed;
  }
  return true;
}


bool MainObject::Export(CatchEvent *evt,QString *err_msg)
{
  bool ret=false;
  RDAudioConvert::ErrorCode conv_err;

  RDCut *cut=new RDCut(evt->cutName());
  if(!cut->exists()) {
    rda->syslog(LOG_WARNING,"cut not found: %s, id: %d",
		(const char *)evt->cutName().toUtf8(),evt->id());
    delete cut;
    return false;
  }
  RDCart *cart=new RDCart(cut->cartNumber());
  RDAudioConvert *conv=new RDAudioConvert(this);
  conv->setSourceFile(RDCut::pathName(evt->cutName()));
  conv->setRange(cut->startPoint(),cut->endPoint());
  conv->setDestinationFile(RDEscapeString(evt->tempName()));
  RDSettings *settings=new RDSettings();
  settings->setFormat((RDSettings::Format)evt->format());
  settings->setChannels(evt->channels());
  settings->setSampleRate(evt->sampleRate());
  settings->setBitRate(evt->bitrate());
  settings->setQuality(evt->quality());
  settings->setNormalizationLevel(evt->normalizeLevel()/100);
  conv->setDestinationSettings(settings);
  RDWaveData *wavedata=NULL;
  if(evt->enableMetadata()) {
    wavedata=new RDWaveData();
    cart->getMetadata(wavedata);
    cut->getMetadata(wavedata);
    conv->setDestinationWaveData(wavedata);
  }
  switch((conv_err=conv->convert())) {
  case RDAudioConvert::ErrorOk:
    ret=true;
    break;

  default:
    rda->syslog(LOG_WARNING,"export error: %s, id: %d",
		(const char *)RDAudioConvert::errorText(conv_err).toUtf8(),
		evt->id());
    ret=false;
    break;
  }
  if(wavedata!=NULL) {
    delete wavedata;
  }
  delete settings;
  delete conv;
  delete cart;
  delete cut;
  return ret;
}


bool MainObject::Import(CatchEvent *evt,QString *err_msg)
{
  bool ret=false;
  RDAudioConvert::ErrorCode conv_err;

  RDCut *cut=new RDCut(evt->cutName());
  if(!cut->exists()) {
    rda->syslog(LOG_WARNING,"cut not found: %s, id: %d",
		(const char *)evt->cutName().toUtf8(),evt->id());
    *err_msg=tr("Cut not found");
    delete cut;
    return false;
  }

  RDWaveFile *wave=new RDWaveFile(evt->tempName());
  if(!wave->openWave()) {
    rda->syslog(LOG_ERR,"unknown file format: %s, id: %d",
		(const char *)evt->cutName().toUtf8(),evt->id());
    *err_msg=tr("Unknown file format");
    delete wave;
    return false;
  }
  unsigned msecs=wave->getExtTimeLength();
  delete wave;
  RDCart *cart=new RDCart(cut->cartNumber());
  RDAudioConvert *conv=new RDAudioConvert(this);
  conv->setSourceFile(RDEscapeString(evt->tempName()));
  conv->setDestinationFile(RDCut::pathName(evt->cutName()));
  RDDeck *deck=new RDDeck(rda->station()->name(),evt->channel());
  RDSettings *settings=new RDSettings();
  settings->setFormat(deck->defaultFormat());
  settings->setChannels(evt->channels());
  settings->setSampleRate(rda->system()->sampleRate());
  settings->setBitRate(deck->defaultBitrate());
  settings->setNormalizationLevel(evt->normalizeLevel()/100);
  rda->syslog(LOG_INFO,"started import of %s to cut %s, id=%d",
	      (const char *)evt->tempName().toUtf8(),
	      (const char *)evt->cutName().toUtf8(),
	 evt->id());
  conv->setDestinationSettings(settings);
  delete deck;

  switch((conv_err=conv->convert())) {
  case RDAudioConvert::ErrorOk:
    CheckInRecording(evt->cutName(),evt,msecs,evt->trimThreshold());
    ret=true;
    break;

  case RDAudioConvert::ErrorFormatNotSupported:
    rda->syslog(LOG_WARNING,"import error: %s, id: %d",
		(const char *)RDAudioConvert::errorText(conv_err).toUtf8(),
		evt->id());
    *err_msg=RDAudioConvert::errorText(conv_err);
    ret=false;
    break;

  default:
    rda->syslog(LOG_WARNING,"import error: %s, id: %d",
		(const char *)RDAudioConvert::errorText(conv_err).toUtf8(),
		evt->id());
    ret=false;
    break;
  }
  if((conv->sourceWaveData()!=NULL)&&(evt->enableMetadata())) {
    cart->setMetadata(conv->sourceWaveData());
    cut->setMetadata(conv->sourceWaveData());
  }
  rda->syslog(LOG_INFO,"completed import of %s to cut %s, id=%d",
	      (const char *)evt->tempName().toUtf8(),
	      (const char *)evt->cutName().toUtf8(),
	      evt->id());
  if(evt->deleteTempFile()) {
    unlink(evt->tempName().toUtf8());
    rda->syslog(LOG_DEBUG,"deleted file \"%s\"",
		(const char *)evt->tempName().toUtf8());
  }
  delete settings;
  delete conv;
  delete cart;
  delete cut;

  return ret;
}


