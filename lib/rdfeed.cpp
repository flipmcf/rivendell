// rdfeed.cpp
//
// Abstract a Rivendell RSS Feed
//
//   (C) Copyright 2002-2020 Fred Gleason <fredg@paravelsystems.com>
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

#include <errno.h>
#include <math.h>

#include <curl/curl.h>

#include <qapplication.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qurl.h>

#include "rdapplication.h"
#include "rdaudioconvert.h"
#include "rdaudioexport.h"
#include "rdcart.h"
#include "rdcut.h"
#include "rdconf.h"
#include "rddb.h"
#include "rddelete.h"
#include "rdescape_string.h"
#include "rdfeed.h"
#include "rdlibrary_conf.h"
#include "rdpodcast.h"
#include "rdtempdirectory.h"
#include "rdupload.h"
#include "rdwavefile.h"

size_t __RDFeed_Readfunction_Callback(char *buffer,size_t size,size_t nitems,
				      void *userdata)
{
  RDFeed *feed=(RDFeed *)userdata;

  int curlsize=size*nitems;
  int segsize=feed->feed_xml.size()-feed->feed_xml_ptr;
  if(segsize<curlsize) {
    curlsize=segsize;
  }
  memcpy(buffer,feed->feed_xml.mid(feed->feed_xml_ptr,curlsize).constData(),
	 curlsize);
  feed->feed_xml_ptr+=curlsize;
  return curlsize;
}

RDFeed::RDFeed(const QString &keyname,RDConfig *config,QObject *parent)
  : QObject(parent)
{
  RDSqlQuery *q;
  QString sql;

  feed_keyname=keyname;
  feed_config=config;

  feed_schemas=new RDRssSchemas();

  sql=QString("select ID from FEEDS where ")+
    "KEY_NAME=\""+RDEscapeString(keyname)+"\"";
  q=new RDSqlQuery(sql);
  if(q->first()) {
    feed_id=q->value(0).toUInt();
  }
  delete q;

  //
  // Get the CGI Hostname
  //
  if(getenv("SERVER_NAME")!=NULL) {
    feed_cgi_hostname=getenv("SERVER_NAME");
  }
}


RDFeed::RDFeed(unsigned id,RDConfig *config,QObject *parent)
  : QObject(parent)
{
  RDSqlQuery *q;
  QString sql;

  feed_id=id;
  feed_config=config;

  feed_schemas=new RDRssSchemas();

  sql=QString().sprintf("select KEY_NAME from FEEDS where ID=%u",id);
  q=new RDSqlQuery(sql);
  if(q->first()) {
    feed_keyname=q->value(0).toString();
  }
  delete q;
}


RDFeed::~RDFeed()
{
  delete feed_schemas;
}


bool RDFeed::exists() const
{
  return RDDoesRowExist("FEEDS","KEY_NAME",feed_keyname);
}


bool RDFeed::isSuperfeed() const
{
  return RDBool(RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"IS_SUPERFEED").
		toString());
  
}


void RDFeed::setIsSuperfeed(bool state) const
{
  SetRow("IS_SUPERFEED",RDYesNo(state));
}


QStringList RDFeed::subfeedNames() const
{
  QString sql;
  RDSqlQuery *q=NULL;
  QStringList ret;

  sql=QString("select ")+
    "MEMBER_KEY_NAME "+  // 00
    "from SUPERFEED_MAPS where "+
    "KEY_NAME=\""+RDEscapeString(keyName())+"\"";
  q=new RDSqlQuery(sql);
  while(q->next()) {
    ret.push_back(q->value(0).toString());
  }
  delete q;

  return ret;
}


QStringList RDFeed::isSubfeedOf() const
{
  QStringList ret;

  QString sql=QString("select ")+
    "KEY_NAME "+  // 00
    "from SUPERFEED_MAPS where "+
    "MEMBER_KEY_NAME=\""+RDEscapeString(keyName())+"\"";
  RDSqlQuery *q=new RDSqlQuery(sql);
  while(q->next()) {
    ret.push_back(q->value(0).toString());
  }
  delete q;

  return ret;
}


bool RDFeed::audienceMetrics() const
{
  return RDBool(RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,
			      "AUDIENCE_METRICS").toString());
}


void RDFeed::setAudienceMetrics(bool state)
{
  SetRow("AUDIENCE_METRICS",RDYesNo(state));
}


QString RDFeed::keyName() const
{
  return feed_keyname;
}


unsigned RDFeed::id() const
{
  return feed_id;
}


QString RDFeed::channelTitle() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_TITLE").
    toString();
}


void RDFeed::setChannelTitle(const QString &str) const
{
  SetRow("CHANNEL_TITLE",str);
}


QString RDFeed::channelDescription() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_DESCRIPTION").
    toString();
}


void RDFeed::setChannelDescription(const QString &str) const
{
  SetRow("CHANNEL_DESCRIPTION",str);
}


QString RDFeed::channelCategory() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_CATEGORY").
    toString();
}


void RDFeed::setChannelCategory(const QString &str) const
{
  SetRow("CHANNEL_CATEGORY",str);
}


QString RDFeed::channelLink() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_LINK").
    toString();
}


void RDFeed::setChannelLink(const QString &str) const
{
  SetRow("CHANNEL_LINK",str);
}


QString RDFeed::channelCopyright() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_COPYRIGHT").
    toString();
}


void RDFeed::setChannelCopyright(const QString &str) const
{
  SetRow("CHANNEL_COPYRIGHT",str);
}


QString RDFeed::channelWebmaster() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_WEBMASTER").
    toString();
}



void RDFeed::setChannelWebmaster(const QString &str) const
{
  SetRow("CHANNEL_WEBMASTER",str);
}


QString RDFeed::channelEditor() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_EDITOR").
    toString();
}


void RDFeed::setChannelEditor(const QString &str) const
{
  SetRow("CHANNEL_EDITOR",str);
}


QString RDFeed::channelAuthor() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_AUTHOR").
    toString();
}


void RDFeed::setChannelAuthor(const QString &str) const
{
  SetRow("CHANNEL_AUTHOR",str);
}


QString RDFeed::channelOwnerName() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_OWNER_NAME").
    toString();
}


void RDFeed::setChannelOwnerName(const QString &str) const
{
  SetRow("CHANNEL_OWNER_NAME",str);
}


QString RDFeed::channelOwnerEmail() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_OWNER_EMAIL").
    toString();
}


void RDFeed::setChannelOwnerEmail(const QString &str) const
{
  SetRow("CHANNEL_OWNER_EMAIL",str);
}


QString RDFeed::channelLanguage() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_LANGUAGE").
    toString();
}


void RDFeed::setChannelLanguage(const QString &str)
{
  SetRow("CHANNEL_LANGUAGE",str);
}


bool RDFeed::channelExplicit() const
{
  return RDBool(RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,
			      "CHANNEL_EXPLICIT").toString());
}


void RDFeed::setChannelExplicit(bool state) const
{
  SetRow("CHANNEL_EXPLICIT",RDYesNo(state));
}


int RDFeed::channelImageId() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_IMAGE_ID").
    toInt();
}


void RDFeed::setChannelImageId(int img_id) const
{
  SetRow("CHANNEL_IMAGE_ID",img_id);
}


int RDFeed::defaultItemImageId() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"DEFAULT_ITEM_IMAGE_ID").
    toInt();
}


void RDFeed::setDefaultItemImageId(int img_id) const
{
  SetRow("DEFAULT_ITEM_IMAGE_ID",img_id);
}


QString RDFeed::baseUrl(const QString &subfeed_key_name) const
{
  QString key_name=subfeed_key_name;
  if(subfeed_key_name.isEmpty()) {
    key_name=feed_keyname;
  }
  return RDGetSqlValue("FEEDS","KEY_NAME",key_name,"BASE_URL").toString();  
}


QString RDFeed::baseUrl(int subfeed_feed_id) const
{
  int id=subfeed_feed_id;
  if(subfeed_feed_id<0) {
    id=feed_id;
  }
  return RDGetSqlValue("FEEDS","ID",id,"BASE_URL").toString();  
}



void RDFeed::setBaseUrl(const QString &str) const
{
  SetRow("BASE_URL",str);
}


QString RDFeed::basePreamble() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"BASE_PREAMBLE").
    toString();
}


void RDFeed::setBasePreamble(const QString &str) const
{
  SetRow("BASE_PREAMBLE",str);
}


QString RDFeed::purgeUrl() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"PURGE_URL").
    toString();
}


void RDFeed::setPurgeUrl(const QString &str) const
{
  SetRow("PURGE_URL",str);
}


QString RDFeed::purgeUsername() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"PURGE_USERNAME").
    toString();
}


void RDFeed::setPurgeUsername(const QString &str) const
{
  SetRow("PURGE_USERNAME",str);
}


QString RDFeed::purgePassword() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"PURGE_PASSWORD").
    toString();
}


void RDFeed::setPurgePassword(const QString &str) const
{
  SetRow("PURGE_PASSWORD",str);
}


RDRssSchemas::RssSchema RDFeed::rssSchema() const
{
  return (RDRssSchemas::RssSchema)RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,
					       "RSS_SCHEMA").toUInt();
}


void RDFeed::setRssSchema(RDRssSchemas::RssSchema schema) const
{
  SetRow("RSS_SCHEMA",(unsigned)schema);
}


QString RDFeed::headerXml() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"HEADER_XML").
    toString();
}


void RDFeed::setHeaderXml(const QString &str)
{
  SetRow("HEADER_XML",str);
}


QString RDFeed::channelXml() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"CHANNEL_XML").
    toString();
}


void RDFeed::setChannelXml(const QString &str)
{
  SetRow("CHANNEL_XML",str);
}


QString RDFeed::itemXml() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"ITEM_XML").
    toString();
}


void RDFeed::setItemXml(const QString &str)
{
  SetRow("ITEM_XML",str);
}


QString RDFeed::feedUrl() const
{
  return purgeUrl()+"/"+keyName()+"."+RD_RSS_XML_FILE_EXTENSION;
}


bool RDFeed::castOrder() const
{
  return RDBool(RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,
			      "CAST_ORDER").toString());
}


void RDFeed::setCastOrder(bool state) const
{
  SetRow("CAST_ORDER",RDYesNo(state));
}


int RDFeed::maxShelfLife() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"MAX_SHELF_LIFE").toInt();
}


void RDFeed::setMaxShelfLife(int days)
{
  SetRow("MAX_SHELF_LIFE",days);
}


QDateTime RDFeed::lastBuildDateTime() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"LAST_BUILD_DATETIME").
    toDateTime();
}


void RDFeed::setLastBuildDateTime(const QDateTime &datetime) const
{
  SetRow("LAST_BUILD_DATETIME",datetime,"yyyy-MM-dd hh:mm:ss");
}


QDateTime RDFeed::originDateTime() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"ORIGIN_DATETIME").
    toDateTime();
}


void RDFeed::setOriginDateTime(const QDateTime &datetime) const
{
  SetRow("ORIGIN_DATETIME",datetime,"yyyy-MM-dd hh:mm:ss");
}


bool RDFeed::enableAutopost() const
{
  return RDBool(RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,
			      "ENABLE_AUTOPOST").toString());
}


void RDFeed::setEnableAutopost(bool state) const
{
  SetRow("ENABLE_AUTOPOST",RDYesNo(state));
}


bool RDFeed::keepMetadata() const
{
  return RDBool(RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,
			      "KEEP_METADATA").toString());
}


void RDFeed::setKeepMetadata(bool state)
{
  SetRow("KEEP_METADATA",RDYesNo(state));
}


RDSettings::Format RDFeed::uploadFormat() const
{
  return (RDSettings::Format)RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,
					   "UPLOAD_FORMAT").toInt();
}


void RDFeed::setUploadFormat(RDSettings::Format fmt) const
{
  SetRow("UPLOAD_FORMAT",(int)fmt);
}


int RDFeed::uploadChannels() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_CHANNELS").
    toInt();
}


void RDFeed::setUploadChannels(int chans) const
{
  SetRow("UPLOAD_CHANNELS",chans);
}


int RDFeed::uploadQuality() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_QUALITY").
    toInt();
}


void RDFeed::setUploadQuality(int qual) const
{
  SetRow("UPLOAD_QUALITY",qual);
}


int RDFeed::uploadBitRate() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_BITRATE").
    toInt();
}


void RDFeed::setUploadBitRate(int rate) const
{
  SetRow("UPLOAD_BITRATE",rate);
}


int RDFeed::uploadSampleRate() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_SAMPRATE").
    toInt();
}


void RDFeed::setUploadSampleRate(int rate) const
{
  SetRow("UPLOAD_SAMPRATE",rate);
}


QString RDFeed::uploadExtension() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_EXTENSION").
    toString();
}


void RDFeed::setUploadExtension(const QString &str)
{
  SetRow("UPLOAD_EXTENSION",str);
}


QString RDFeed::uploadMimetype() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"UPLOAD_MIMETYPE").
    toString();
}


void RDFeed::setUploadMimetype(const QString &str)
{
  SetRow("UPLOAD_MIMETYPE",str);
}


int RDFeed::normalizeLevel() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"NORMALIZE_LEVEL").
    toInt();
}


void RDFeed::setNormalizeLevel(int lvl) const
{
  SetRow("NORMALIZE_LEVEL",lvl);
}


QString RDFeed::redirectPath() const
{
  return RDGetSqlValue("FEEDS","KEY_NAME",feed_keyname,"REDIRECT_PATH").
    toString();
}


void RDFeed::setRedirectPath(const QString &str)
{
  SetRow("REDIRECT_PATH",str);
}


RDFeed::MediaLinkMode RDFeed::mediaLinkMode() const
{
  return (RDFeed::MediaLinkMode)RDGetSqlValue("FEEDS","KEY_NAME",
					      feed_keyname,"MEDIA_LINK_MODE").
    toUInt();
}
  

void RDFeed::setMediaLinkMode(RDFeed::MediaLinkMode mode) const
{
  SetRow("MEDIA_LINK_MODE",(unsigned)mode);
}


int RDFeed::importImageFile(const QString &pathname,QString *err_msg,
			    QString desc) const
{
  bool ok=false;
  QString sql;
  int ret;
  QSize min=rssSchemas()->minimumImageSize(rssSchema());
  QSize max=rssSchemas()->maximumImageSize(rssSchema());
  *err_msg="OK";

  //
  // Load the image
  //
  QFile file(pathname);
  if(!file.open(QIODevice::ReadOnly)) {
    *err_msg=QString("Unable to open image file [")+
      QString(strerror(errno))+"]";
    return -1;
  }
  QByteArray data=file.readAll();
  file.close();

  //
  // Validate the image
  //
  QImage *img=new QImage();
  if(!img->loadFromData(data)) {
    *err_msg="Invalid image file!";
    return -1;
  }
  if((!min.isNull())&&
     ((img->width()<min.width())||(img->height()<min.height()))) {
    *err_msg=
      QString().sprintf("Image is too small - %dx%d or larger required",
			min.width(),min.height());
    return -1;
  }
  if((!max.isNull())&&
     ((img->width()>max.width())||(img->height()>max.height()))) {
    *err_msg=
      QString().sprintf("Image is too large - %dx%d or smaller required",
			max.width(),max.height());
    return -1;
  }

  //
  // Fix up the Description
  //
  if(desc.isEmpty()) {
    desc=tr("Imported from")+" "+pathname;
  }

  //
  // FIXME: Upload to remote file store here...
  //

  //
  // Write it to the DB
  //
  QStringList f0=pathname.split(".",QString::SkipEmptyParts);
  sql=QString("insert into FEED_IMAGES set ")+
    QString().sprintf("FEED_ID=%u,",id())+
    "FEED_KEY_NAME=\""+RDEscapeString(keyName())+"\","+
    QString().sprintf("WIDTH=%d,",img->width())+
    QString().sprintf("HEIGHT=%d,",img->height())+
    QString().sprintf("DEPTH=%d,",img->depth())+
    "DESCRIPTION=\""+RDEscapeString(desc)+"\","+
    "FILE_EXTENSION=\""+RDEscapeString(f0.last().toLower())+"\","+
    "DATA="+RDEscapeBlob(data);
  ret=RDSqlQuery::run(sql,&ok).toInt();
  if(!ok) {
    *err_msg="Unable to write to database";
    return -1;
  }

  return ret;
}


bool RDFeed::deleteImage(int img_id,QString *err_msg)
{
  QString sql;
  RDSqlQuery *q=NULL;

  *err_msg="OK";

  //
  // FIXME: Delete from remote file store here...
  //

  sql=QString("delete from FEED_IMAGES where ")+
    QString().sprintf("ID=%d",img_id);
  if(!RDSqlQuery::apply(sql,err_msg)) {
    *err_msg=QString("database error: ")+*err_msg;
    delete q;
    return false;
  }
  delete q;

  return true;
}


QString RDFeed::audioUrl(RDFeed::MediaLinkMode mode,
			 const QString &cgi_hostname,unsigned cast_id)
{
  RDPodcast *cast=new RDPodcast(feed_config,cast_id);
  QUrl url(baseUrl(cast->feedId()));
  QString ret;

  switch(mode) {
  case RDFeed::LinkNone:
    ret="";
    break;

  case RDFeed::LinkDirect:
    cast=new RDPodcast(feed_config,cast_id);
    ret=url.toString()+"/"+cast->audioFilename();
    break;

  case RDFeed::LinkCounted:
    ret=QString("http://")+basePreamble()+cgi_hostname+
      "/rd-bin/rdfeed."+uploadExtension()+"?"+keyName()+
      QString().sprintf("&cast_id=%d",cast_id);
    break;
  }
  delete cast;

  return ret;
}


QString RDFeed::imageUrl(int img_id) const
{
  QString ret;

  QString sql=QString("select ")+
    "FEED_ID,"+         // 00
    "FILE_EXTENSION "+  // 01
    "from FEED_IMAGES where "+
    QString().sprintf("ID=%d",img_id);
  RDSqlQuery *q=new RDSqlQuery(sql);
  if(q->first()) {
    ret=baseUrl(q->value(0).toUInt())+"/"+
      RDFeed::imageFilename(id(),img_id,q->value(1).toString());
  }
  delete q;

  return ret;
}


bool RDFeed::postXml(QString *err_msg)
{
  CURL *curl=NULL;
  CURLcode curl_err;
  bool ret=false;
  char errstr[CURL_ERROR_SIZE];

  if((curl=curl_easy_init())==NULL) {
    *err_msg=tr("Unable to get CURL handle.");
    return false;
  }

  feed_xml=rssXml(err_msg).toUtf8();
  feed_xml_ptr=0;

  curl_easy_setopt(curl,CURLOPT_URL,feedUrl().toUtf8().constData());
  curl_easy_setopt(curl,CURLOPT_UPLOAD,1);
  curl_easy_setopt(curl,CURLOPT_READFUNCTION, __RDFeed_Readfunction_Callback);
  curl_easy_setopt(curl,CURLOPT_READDATA,this);
  curl_easy_setopt(curl,CURLOPT_USERNAME,purgeUsername().toUtf8().constData());
  curl_easy_setopt(curl,CURLOPT_PASSWORD,purgePassword().toUtf8().constData());

  curl_easy_setopt(curl,CURLOPT_TIMEOUT,RD_CURL_TIMEOUT);
  curl_easy_setopt(curl,CURLOPT_NOPROGRESS,1);
  curl_easy_setopt(curl,CURLOPT_USERAGENT,
		   (const char *)rda->config()->userAgent().utf8());
  curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,errstr);
  /*
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curl,CURLOPT_DEBUGFUNCTION,UploadErrorCallback);
  */
  switch((curl_err=curl_easy_perform(curl))) {
  case CURLE_OK:
  case CURLE_PARTIAL_FILE:
    ret=true;
    break;

  default:
    *err_msg=errstr;
    ret=false;
    break;
  }
  curl_easy_cleanup(curl);

  //
  // Update Enclosing Superfeeds
  //
  QStringList superfeeds=isSubfeedOf();
  for(int i=0;i<superfeeds.size();i++) {
    QString err_msg2;
    RDFeed *feed=new RDFeed(superfeeds.at(i),feed_config,this);
    if(!feed->postXml(&err_msg2)) {
      *err_msg+="\n"+err_msg2;
    }
    delete feed;
  }

  return ret;
}


bool RDFeed::postXmlConditional(const QString &caption,QWidget *widget)
{
  QString err_msg;

  if(!audienceMetrics()) {
    if(!postXml(&err_msg)) {
      QMessageBox::warning(widget,caption+" - "+tr("Error"),
			   tr("XML data upload failed!")+"\n"+
			   "["+err_msg+"]");
      return false;
    }
  }

  return true;
}


bool RDFeed::deleteXml(QString *err_msg)
{
  RDDelete::ErrorCode conv_err;
  RDDelete *conv=new RDDelete(rda->config());
  if(!conv->urlIsSupported(feedUrl())) {
    *err_msg="unsupported url scheme";
    delete conv;
    return false;
  }
  conv->setTargetUrl(feedUrl());
  conv_err=conv->runDelete(purgeUsername(),purgePassword(),
			   rda->config()->logXloadDebugData());
  *err_msg=RDDelete::errorText(conv_err);
  delete conv;

  return conv_err==RDDelete::ErrorOk;
}


bool RDFeed::deleteImages(QString *err_msg)
{
  RDDelete::ErrorCode conv_err=RDDelete::ErrorOk;

  QString sql=QString("select ")+
    "ID,"+              // 00
    "FILE_EXTENSION "+  // 01
    "from FEED_IMAGES where "+
    "FEED_KEY_NAME=\""+RDEscapeString(keyName())+"\"";
  RDSqlQuery *q=new RDSqlQuery(sql);
  while(q->next()) {
    QString img_url=purgeUrl()+"/"+
      RDFeed::imageFilename(id(),q->value(0).toInt(),q->value(1).toString());
    RDDelete *conv=new RDDelete(rda->config());
    if(!conv->urlIsSupported(img_url)) {
      *err_msg="unsupported url scheme";
      delete conv;
      return false;
    }
    conv->setTargetUrl(img_url);
    conv_err=conv->runDelete(purgeUsername(),purgePassword(),
			     rda->config()->logXloadDebugData());
    *err_msg=RDDelete::errorText(conv_err);
    delete conv;
  }
  delete q;

  return conv_err==RDDelete::ErrorOk;
}


unsigned RDFeed::postCut(RDUser *user,RDStation *station,
			 const QString &cutname,Error *err,bool log_debug,
			 RDConfig *config)
{
  QString err_msg;
  QString tmpfile;
  QString destfile;
  QString sql;
  RDSqlQuery *q;
  RDPodcast *cast=NULL;
  RDUpload *upload=NULL;
  RDUpload::ErrorCode upload_err;
  RDAudioConvert::ErrorCode audio_conv_err;
  RDAudioExport::ErrorCode export_err;

  emit postProgressChanged(0);
  emit postProgressChanged(1);

  //
  // Export Cut
  //
  tmpfile=GetTempFilename();
  RDCut *cut=new RDCut(cutname);
  if(!cut->exists()) {
    delete cut;
    *err=RDFeed::ErrorCannotOpenFile;
    return 0;
  }
  RDAudioExport *conv=new RDAudioExport(this);
  conv->setCartNumber(cut->cartNumber());
  conv->setCutNumber(cut->cutNumber());
  conv->setDestinationFile(tmpfile);
  conv->setRange(cut->startPoint(),cut->endPoint());
  RDSettings *settings=new RDSettings();
  settings->setFormat(uploadFormat());
  settings->setChannels(uploadChannels());
  settings->setSampleRate(uploadSampleRate());
  settings->setBitRate(uploadBitRate());
  settings->setNormalizationLevel(normalizeLevel()/100);
  conv->setDestinationSettings(settings);
  switch((export_err=conv->runExport(user->name(),user->password(),&audio_conv_err))) {
  case RDAudioExport::ErrorOk:
    break;

  case RDAudioExport::ErrorInvalidSettings:
    delete settings;
    delete conv;
    *err=RDFeed::ErrorUnsupportedType;
    unlink(tmpfile);
    return 0;

  case RDAudioExport::ErrorNoSource:
  case RDAudioExport::ErrorNoDestination:
  case RDAudioExport::ErrorInternal:
  case RDAudioExport::ErrorUrlInvalid:
  case RDAudioExport::ErrorService:
  case RDAudioExport::ErrorInvalidUser:
  case RDAudioExport::ErrorAborted:
  case RDAudioExport::ErrorConverter:
    delete settings;
    delete conv;
    *err=RDFeed::ErrorGeneral;
    unlink(tmpfile);
    return 0;
  }
  delete settings;
  delete conv;

  //
  // Upload
  //
  emit postProgressChanged(2);
  QFile file(tmpfile);
  int length=file.size();
  unsigned cast_id=CreateCast(&destfile,length,cut->length());
  delete cut;
  cast=new RDPodcast(feed_config,cast_id);
  upload=new RDUpload(rda->config(),this);
  upload->setSourceFile(tmpfile);
  upload->setDestinationUrl(purgeUrl()+"/"+cast->audioFilename());
  switch((upload_err=upload->runUpload(purgeUsername(),purgePassword(),
				       log_debug))) {
  case RDUpload::ErrorOk:
    *err=RDFeed::ErrorOk;
    break;

  default:
    emit postProgressChanged(totalPostSteps());
    *err=RDFeed::ErrorUploadFailed;
    sql=QString().sprintf("delete from PODCASTS where ID=%u",cast_id);
    q=new RDSqlQuery(sql);
    delete q;
    delete upload;
    delete cast;
    *err=RDFeed::ErrorUploadFailed;
    unlink(tmpfile);
    return 0;
  }
  emit postProgressChanged(3);
  unlink(tmpfile);
  delete upload;

  //
  // Set default cast parameters
  //
  cast->setItemAuthor(user->emailContact());
  cast->setItemImageId(defaultItemImageId());
  delete cast;

  if(!audienceMetrics()) {
    emit postProgressChanged(4);
    postXml(&err_msg);
  }

  emit postProgressChanged(totalPostSteps());

  return cast_id;
}


unsigned RDFeed::postFile(RDUser *user,RDStation *station,
			  const QString &srcfile,Error *err,bool log_debug,
			  RDConfig *config)
{
  QString err_msg;
  QString sql;
  RDSqlQuery *q;
  QString cmd;
  QString tmpfile;
  QString tmpfile2;
  QString destfile;
  int time_length=0;
  RDUpload *upload=NULL;
  RDUpload::ErrorCode upload_err;
  RDWaveFile *wave=NULL;
  unsigned audio_time=0;

  emit postProgressChanged(0);
  emit postProgressChanged(1);
  qApp->processEvents();

  //
  // Convert Cut
  //
  tmpfile=GetTempFilename();
  RDAudioConvert *conv=new RDAudioConvert(this);
  conv->setSourceFile(srcfile);
  conv->setDestinationFile(tmpfile);
  RDSettings *settings=new RDSettings();
  settings->setFormat(uploadFormat());
  settings->setChannels(uploadChannels());
  settings->setSampleRate(uploadSampleRate());
  settings->setBitRate(uploadBitRate());
  settings->setNormalizationLevel(normalizeLevel()/100);
  conv->setDestinationSettings(settings);
  switch(conv->convert()) {
  case RDAudioConvert::ErrorOk:
    wave=new RDWaveFile(tmpfile);
    if(wave->openWave()) {
      audio_time=wave->getExtTimeLength();
    }
    delete wave;
    break;

  case RDAudioConvert::ErrorInvalidSettings:
  case RDAudioConvert::ErrorFormatNotSupported:
    emit postProgressChanged(totalPostSteps());
    delete settings;
    delete conv;
    *err=RDFeed::ErrorUnsupportedType;
    unlink(tmpfile);
    return 0;

  case RDAudioConvert::ErrorNoSource:
  case RDAudioConvert::ErrorNoDestination:
  case RDAudioConvert::ErrorInternal:
  case RDAudioConvert::ErrorInvalidSource:
  case RDAudioConvert::ErrorNoDisc:
  case RDAudioConvert::ErrorNoTrack:
  case RDAudioConvert::ErrorInvalidSpeed:
  case RDAudioConvert::ErrorFormatError:
  case RDAudioConvert::ErrorNoSpace:
    emit postProgressChanged(totalPostSteps());
    delete settings;
    delete conv;
    *err=RDFeed::ErrorGeneral;
    unlink(tmpfile);
    return 0;
  }
  delete settings;
  delete conv;

  //
  // Upload
  //
  emit postProgressChanged(2);
  emit postProgressChanged(3);
  qApp->processEvents();
  QFile file(tmpfile);
  int length=file.size();

  unsigned cast_id=CreateCast(&destfile,length,time_length);
  RDPodcast *cast=new RDPodcast(feed_config,cast_id);
  upload=new RDUpload(rda->config(),this);
  upload->setSourceFile(tmpfile);
  upload->setDestinationUrl(purgeUrl()+"/"+cast->audioFilename());
  switch((upload_err=upload->runUpload(purgeUsername(),purgePassword(),
				       log_debug))) {
  case RDUpload::ErrorOk:
    sql=QString().sprintf("update PODCASTS set AUDIO_TIME=%u where ID=%u",
			  audio_time,cast_id);
    q=new RDSqlQuery(sql);
    delete q;
    break;

  default:
    emit postProgressChanged(totalPostSteps());
    *err=RDFeed::ErrorUploadFailed;
    sql=QString().sprintf("delete from PODCASTS where ID=%u",cast_id);
    q=new RDSqlQuery(sql);
    delete q;
    delete upload;
    delete cast;
    *err=RDFeed::ErrorUploadFailed;
    unlink(tmpfile);
    return 0;
  }
  delete upload;

  unlink(QString(tmpfile)+".wav");
  unlink(tmpfile);

  //
  // Set default cast parameters
  //
  cast->setItemAuthor(user->emailContact());
  cast->setItemImageId(defaultItemImageId());
  delete cast;

  if(!audienceMetrics()) {
    emit postProgressChanged(4);
    postXml(&err_msg);
  }

  emit postProgressChanged(totalPostSteps());

  *err=RDFeed::ErrorOk;
  return cast_id;
}


int RDFeed::totalPostSteps() const
{
  if(audienceMetrics()) {
    return RDFEED_TOTAL_POST_STEPS;
  }
  return RDFEED_TOTAL_POST_STEPS+1;
}


QString RDFeed::rssXml(QString *err_msg,bool *ok)
{
  QString ret;

  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;

  if(ok!=NULL) {
    *ok=false;
  }
  sql=QString("select ")+
    "FEEDS.CHANNEL_TITLE,"+        // 00
    "FEEDS.CHANNEL_DESCRIPTION,"+  // 01
    "FEEDS.CHANNEL_CATEGORY,"+     // 02
    "FEEDS.CHANNEL_LINK,"+         // 03
    "FEEDS.CHANNEL_COPYRIGHT,"+    // 04
    "FEEDS.CHANNEL_EDITOR,"+       // 05
    "FEEDS.CHANNEL_AUTHOR,"+       // 06
    "FEEDS.CHANNEL_OWNER_NAME,"+   // 07
    "FEEDS.CHANNEL_OWNER_EMAIL,"+  // 08
    "FEEDS.CHANNEL_WEBMASTER,"+    // 09
    "FEEDS.CHANNEL_LANGUAGE,"+     // 10
    "FEEDS.CHANNEL_EXPLICIT,"+     // 11
    "FEEDS.LAST_BUILD_DATETIME,"+  // 12
    "FEEDS.ORIGIN_DATETIME,"+      // 13
    "FEEDS.HEADER_XML,"+           // 14
    "FEEDS.CHANNEL_XML,"+          // 15
    "FEEDS.ITEM_XML,"+             // 16
    "FEEDS.BASE_URL,"+             // 17
    "FEEDS.ID,"+                   // 18
    "FEEDS.UPLOAD_EXTENSION,"+     // 19
    "FEEDS.CAST_ORDER,"+           // 20
    "FEEDS.REDIRECT_PATH,"+        // 21
    "FEEDS.BASE_PREAMBLE,"+        // 22
    "FEEDS.AUDIENCE_METRICS,"+     // 23
    "FEEDS.IS_SUPERFEED,"+         // 24
    "FEED_IMAGES.ID,"+             // 25
    "FEED_IMAGES.WIDTH,"+          // 26
    "FEED_IMAGES.HEIGHT,"+         // 27
    "FEED_IMAGES.DESCRIPTION,"+    // 28
    "FEED_IMAGES.FILE_EXTENSION "+ // 29
    "from FEEDS ";
  sql+="left join FEED_IMAGES ";
  sql+="on FEEDS.CHANNEL_IMAGE_ID=FEED_IMAGES.ID ";
  sql+="where ";
  sql+="FEEDS.KEY_NAME=\""+RDEscapeString(keyName())+"\"";
  q=new RDSqlQuery(sql);
  if(!q->first()) {
    *err_msg="no feed matches the supplied key name";
    return QString();
  }

  //
  // Load the XML Templates
  //
  QString header_template=rssSchemas()->headerTemplate(rssSchema());
  QString channel_template=rssSchemas()->channelTemplate(rssSchema());
  QString item_template=rssSchemas()->itemTemplate(rssSchema());
  if(rssSchema()==RDRssSchemas::CustomSchema) {
    header_template=q->value(14).toString();
    channel_template=q->value(15).toString();
    item_template=q->value(16).toString();
  }

  //
  // Render Header XML
  //
  ret+=header_template+"\r\n";

  //
  // Render Channel XML
  //
  ret+="  <channel>\n";
  ret+=ResolveChannelWildcards(channel_template,q)+"\r\n";

  //
  // Render Item XML
  //
  QString where;
  if(q->value(24).toString()=="Y") {  // Is a Superfeed
    sql=QString("select ")+
      "MEMBER_FEED_ID "+  // 00
      "from SUPERFEED_MAPS where "+
      QString().sprintf("FEED_ID=%d",q->value(18).toUInt());
    q1=new RDSqlQuery(sql);
    while(q1->next()) {
      where+=QString().sprintf("(PODCASTS.FEED_ID=%u) || ",q1->value(0).toUInt());
    }
    delete q1;
    where=("("+where.left(where.length()-4)+") && ");
  }
  else {
    where =QString().sprintf("(PODCASTS.FEED_ID=%u)&&",q->value(18).toUInt());
  }
  sql=QString("select ")+
    "PODCASTS.FEED_ID,"+             // 00
    "PODCASTS.ITEM_TITLE,"+          // 01
    "PODCASTS.ITEM_DESCRIPTION,"+    // 02
    "PODCASTS.ITEM_CATEGORY,"+       // 03
    "PODCASTS.ITEM_LINK,"+           // 04
    "PODCASTS.ITEM_AUTHOR,"+         // 05
    "PODCASTS.ITEM_SOURCE_TEXT,"+    // 06
    "PODCASTS.ITEM_SOURCE_URL,"+     // 07
    "PODCASTS.ITEM_COMMENTS,"+       // 08
    "PODCASTS.ITEM_EXPLICIT,"+       // 09
    "PODCASTS.AUDIO_FILENAME,"+      // 10
    "PODCASTS.AUDIO_LENGTH,"+        // 11
    "PODCASTS.AUDIO_TIME,"+          // 12
    "PODCASTS.EFFECTIVE_DATETIME,"+  // 13
    "PODCASTS.ID,"+                  // 14
    "FEEDS.BASE_URL,"+               // 15
    "FEEDS.CHANNEL_TITLE,"+          // 16
    "FEEDS.CHANNEL_DESCRIPTION,"+    // 17
    "FEED_IMAGES.ID,"+               // 18
    "FEED_IMAGES.WIDTH,"+            // 19
    "FEED_IMAGES.HEIGHT,"+           // 20
    "FEED_IMAGES.DESCRIPTION,"+      // 21
    "FEED_IMAGES.FILE_EXTENSION "+   // 22
    "from PODCASTS left join FEEDS "+
    "on PODCASTS.FEED_ID=FEEDS.ID "+
    "left join FEED_IMAGES "+
    "on PODCASTS.ITEM_IMAGE_ID=FEED_IMAGES.ID where "+
    where+
    QString().sprintf("(PODCASTS.STATUS=%d) ",RDPodcast::StatusActive)+
    "order by PODCASTS.ORIGIN_DATETIME";
  if(q->value(20).toString()=="N") {
    sql+=" desc";
  }
  q1=new RDSqlQuery(sql);
  while(q1->next()) {
    ret+="    <item>\r\n";
    ret+=ResolveItemWildcards(item_template,q1,q);
    ret+="\r\n";
    ret+="    </item>\r\n";
  }
  delete q1;

  ret+="  </channel>\r\n";
  ret+="</rss>\r\n";
  delete q;

  if(ok!=NULL) {
    *ok=true;
  }

  return ret;
}


RDRssSchemas *RDFeed::rssSchemas() const
{
  return feed_schemas;
}


unsigned RDFeed::create(const QString &keyname,bool enable_users,
			QString *err_msg)
{
  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;
  unsigned feed_id=0;

  //
  // Sanity Checks
  //
  sql=QString("select KEY_NAME from FEEDS where ")+
    "KEY_NAME=\""+RDEscapeString(keyname)+"\"";
  q=new RDSqlQuery(sql);
  if(q->first()) {
    *err_msg=tr("A feed with that key name already exists!");
    delete q;
    return 0;
  }
  delete q;

  //
  // Create Feed
  //
  sql=QString("insert into FEEDS set ")+
    "KEY_NAME=\""+RDEscapeString(keyname)+"\","+
    "ORIGIN_DATETIME=now(),"+
    "HEADER_XML=\"\","+
    "CHANNEL_XML=\"\","+
    "ITEM_XML=\"\"";
  q=new RDSqlQuery(sql);
  feed_id=q->lastInsertId().toUInt();
  delete q;

  //
  // Create Default Feed Perms
  //
  if(enable_users) {
    sql=QString("select LOGIN_NAME from USERS where ")+
      "(ADMIN_USERS_PRIV='N')&&(ADMIN_CONFIG_PRIV='N')";
    q=new RDSqlQuery(sql);
    while(q->next()) {
      sql=QString("insert into FEED_PERMS set ")+
	"USER_NAME=\""+RDEscapeString(q->value(0).toString())+"\","+
	"KEY_NAME=\""+RDEscapeString(keyname)+"\"";
      q1=new RDSqlQuery(sql);
      delete q1;
    }
    delete q;
  }

  return feed_id;
}


QString RDFeed::errorString(RDFeed::Error err)
{
  QString ret="Unknown Error";

  switch(err) {
  case RDFeed::ErrorOk:
    ret="Ok";
    break;

  case RDFeed::ErrorNoFile:
    ret="No such file or directory";
    break;

  case RDFeed::ErrorCannotOpenFile:
    ret="Cannot open file";
    break;

  case RDFeed::ErrorUnsupportedType:
    ret="Unsupported file format";
    break;

  case RDFeed::ErrorUploadFailed:
    ret="Upload failed";
    break;

  case RDFeed::ErrorGeneral:
    ret="General Error";
    break;
  }
  return ret;
}


QString RDFeed::imageFilename(int feed_id,int img_id,const QString &ext)
{
  return QString().sprintf("img%06d_%06d.",feed_id,img_id)+ext;
}


QString RDFeed::publicUrl(const QString &base_url,const QString &keyname)
{
  return base_url+"/"+keyname+"."+RD_RSS_XML_FILE_EXTENSION;
}


unsigned RDFeed::CreateCast(QString *filename,int bytes,int msecs) const
{
  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;
  unsigned cast_id=0;

  sql=QString("select ")+
    "CHANNEL_TITLE,"+        // 00
    "CHANNEL_DESCRIPTION,"+  // 01
    "CHANNEL_CATEGORY,"+     // 02
    "CHANNEL_LINK,"+         // 03
    "MAX_SHELF_LIFE,"+       // 04
    "UPLOAD_FORMAT,"+        // 05
    "UPLOAD_EXTENSION "+     // 06
    "from FEEDS where "+
    QString().sprintf("ID=%u",feed_id);
  q=new RDSqlQuery(sql);
  if(!q->first()) {
    delete q;
    return 0;
  }

  //
  // Create Entry
  //
  sql=QString("insert into PODCASTS set ")+
    QString().sprintf("FEED_ID=%u,",feed_id)+
    "ITEM_TITLE=\""+RDEscapeString(q->value(0).toString())+"\","+
    "ITEM_DESCRIPTION=\""+RDEscapeString(q->value(1).toString())+"\","+
    "ITEM_CATEGORY=\""+RDEscapeString(q->value(2).toString())+"\","+
    "ITEM_LINK=\""+RDEscapeString(q->value(3).toString())+"\","+
    QString().sprintf("SHELF_LIFE=%d,",q->value(4).toInt())+
    "ITEM_AUTHOR=\""+RDEscapeString(rda->user()->emailContact())+"\","+
    "EFFECTIVE_DATETIME=UTC_TIMESTAMP(),"+
    "ORIGIN_DATETIME=UTC_TIMESTAMP()";
  q1=new RDSqlQuery(sql);
  delete q1;

  //
  // Get The Cast ID
  //
  sql="select LAST_INSERT_ID() from PODCASTS";
  q1=new RDSqlQuery(sql);
  if(q1->first()) {
    cast_id=q1->value(0).toUInt();
  }
  delete q1;

  //
  // Generate the Filename
  //
  *filename=
    QString().sprintf("%06u_%06u",feed_id,cast_id)+"."+q->value(6).toString();
  sql=QString("update PODCASTS set ")+
    "AUDIO_FILENAME=\""+RDEscapeString(*filename)+"\","+
    QString().sprintf("AUDIO_LENGTH=%d,",bytes)+
    QString().sprintf("AUDIO_TIME=%d where ",msecs)+
    QString().sprintf("ID=%u",cast_id);
  q1=new RDSqlQuery(sql);
  delete q1;
  delete q;
  return cast_id;
}


QString RDFeed::ResolveChannelWildcards(const QString &tmplt,RDSqlQuery *chan_q)
{
  QString ret="    "+tmplt;

  ret.replace("\n","\r\n    ");
  ret.replace("%TITLE%",RDXmlEscape(chan_q->value(0).toString()));
  ret.replace("%DESCRIPTION%",RDXmlEscape(chan_q->value(1).toString()));
  ret.replace("%CATEGORY%",RDXmlEscape(chan_q->value(2).toString()));
  ret.replace("%LINK%",RDXmlEscape(chan_q->value(3).toString()));
  ret.replace("%COPYRIGHT%",RDXmlEscape(chan_q->value(4).toString()));
  ret.replace("%EDITOR%",RDXmlEscape(chan_q->value(5).toString()));
  ret.replace("%AUTHOR%",RDXmlEscape(chan_q->value(6).toString()));
  ret.replace("%OWNER_NAME%",RDXmlEscape(chan_q->value(7).toString()));
  ret.replace("%OWNER_EMAIL%",RDXmlEscape(chan_q->value(8).toString()));
  ret.replace("%WEBMASTER%",RDXmlEscape(chan_q->value(9).toString()));
  ret.replace("%LANGUAGE%",RDXmlEscape(chan_q->value(10).toString()));
  QString explicit_str="false";
  if(chan_q->value(11).toString()=="Y") {
    explicit_str="true";
  }
  ret.replace("%EXPLICIT%",RDXmlEscape(explicit_str));
  ret.replace("%BUILD_DATE%",chan_q->value(12).toDateTime().
	      toString("ddd, d MMM yyyy hh:mm:ss ")+"GMT");
  ret.replace("%PUBLISH_DATE%",chan_q->value(13).toDateTime().
	      toString("ddd, d MMM yyyy hh:mm:ss ")+"GMT");
  ret.replace("%GENERATOR%",QString("Rivendell ")+VERSION);
  ret.replace("%FEED_URL%",RDXmlEscape(chan_q->value(17).toString())+"/"+
	      RDXmlEscape(keyName()+"."+RD_RSS_XML_FILE_EXTENSION));
  ret.replace("%IMAGE_URL%",chan_q->value(17).toString()+"/"+
	      RDFeed::imageFilename(id(),chan_q->value(25).toInt(),
				    chan_q->value(29).toString()));
  ret.replace("%IMAGE_WIDTH%",
	      QString().sprintf("%d",chan_q->value(26).toInt()));
  ret.replace("%IMAGE_HEIGHT%",
	      QString().sprintf("%d",chan_q->value(26).toInt()));
  ret.replace("%IMAGE_DESCRIPTION%",chan_q->value(28).toString());

  return ret;
}


QString RDFeed::ResolveItemWildcards(const QString &tmplt,RDSqlQuery *item_q,
				     RDSqlQuery *chan_q)
{
  QString ret="      "+tmplt;

  ret.replace("\n","\r\n      ");

  ret.replace("%ITEM_CHANNEL_TITLE%",RDXmlEscape(item_q->value(16).toString()));
  ret.replace("%ITEM_CHANNEL_DESCRIPTION%",
	      RDXmlEscape(item_q->value(17).toString()));
  ret.replace("%ITEM_TITLE%",RDXmlEscape(item_q->value(1).toString()));
  ret.replace("%ITEM_DESCRIPTION%",
	      RDXmlEscape(item_q->value(2).toString()));
  ret.replace("%ITEM_CATEGORY%",
	      RDXmlEscape(item_q->value(3).toString()));
  ret.replace("%ITEM_LINK%",RDXmlEscape(item_q->value(4).toString()));
  ret.replace("%ITEM_AUTHOR%",RDXmlEscape(item_q->value(5).toString()));
  if(chan_q->value(1).toString()=="Y") {  // Audience Metrics
    ret.replace("%ITEM_SOURCE_TEXT%",
		RDXmlEscape(item_q->value(6).toString()));
    ret.replace("%ITEM_SOURCE_URL%",
		RDXmlEscape(item_q->value(7).toString()));
  }
  else {
    ret.replace("%ITEM_SOURCE_TEXT%",
		RDXmlEscape(chan_q->value(0).toString()));
    ret.replace("%ITEM_SOURCE_URL%",
		RDXmlEscape(item_q->value(15).toString()+"/"+keyName()));    
  }
  ret.replace("%ITEM_COMMENTS%",
	      RDXmlEscape(item_q->value(8).toString()));
  QString explicit_str="false";
  if(item_q->value(9).toString()=="Y") {
    explicit_str="true";
  }
  ret.replace("%ITEM_EXPLICIT%",explicit_str);
  if(chan_q->value(23).toString()=="Y") {
    ret.replace("%ITEM_AUDIO_URL%",
		RDXmlEscape(audioUrl(RDFeed::LinkCounted,feed_cgi_hostname,
				     item_q->value(14).toUInt())));
  }
  else {
    ret.replace("%ITEM_AUDIO_URL%",
		RDXmlEscape(audioUrl(RDFeed::LinkDirect,feed_cgi_hostname,
				     item_q->value(14).toUInt())));
  }
  ret.replace("%ITEM_AUDIO_LENGTH%",item_q->value(11).toString());
  ret.replace("%ITEM_AUDIO_TIME%",
	      RDGetTimeLength(item_q->value(12).toInt(),false,false));
  ret.replace("%ITEM_AUDIO_SECONDS%",
	      QString().sprintf("%d",item_q->value(12).toInt()/1000));
  ret.replace("%ITEM_PUBLISH_DATE%",item_q->value(13).toDateTime().
	      toString("ddd, d MMM yyyy hh:mm:ss ")+"GMT");
  ret.replace("%ITEM_GUID%",RDPodcast::guid(item_q->value(15).toString(),
					    item_q->value(10).toString(),
					    item_q->value(0).toUInt(),
					    item_q->value(14).toUInt()));
  ret.replace("%ITEM_IMAGE_URL%",item_q->value(15).toString()+"/"+
	      RDFeed::imageFilename(item_q->value(0).toInt(),
				    item_q->value(18).toInt(),
				    item_q->value(22).toString()));
  return ret;
}


QString RDFeed::GetTempFilename() const
{
  char tempname[PATH_MAX];

  sprintf(tempname,"%s/podcastXXXXXX",(const char *)RDTempDirectory::basePath());
  if(mkstemp(tempname)<0) {
    return QString();
  }

  return QString(tempname);
}


void RDFeed::SetRow(const QString &param,int value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString("update FEEDS set ")+
    param+QString().sprintf("=%d where ",value)+
    "KEY_NAME=\""+RDEscapeString(feed_keyname)+"\"";
  q=new RDSqlQuery(sql);
  delete q;
}


void RDFeed::SetRow(const QString &param,const QString &value) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString("update FEEDS set ")+
    param+"=\""+RDEscapeString(value)+"\" where "+
    "KEY_NAME=\""+RDEscapeString(feed_keyname)+"\"";
  q=new RDSqlQuery(sql);
  delete q;
}

void RDFeed::SetRow(const QString &param,const QDateTime &value,
                    const QString &format) const
{
  RDSqlQuery *q;
  QString sql;

  sql=QString().sprintf("update FEEDS set ")+
    param+"="+RDCheckDateTime(value,format)+" where "+
    "KEY_NAME=\""+RDEscapeString(feed_keyname)+"\"";
  q=new RDSqlQuery(sql);
  delete q;
}
