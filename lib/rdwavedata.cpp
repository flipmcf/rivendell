//   rdwavedata.cpp
//
//   A Container Class for Audio Meta Data.
//
//   (C) Copyright 2002-2024 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License 
//   version 2 as published by the Free Software Foundation.
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

#include <QObject>

#include "rdconf.h"
#include "rdwavedata.h"

RDWaveData::RDWaveData()
{
  clear();
}


bool RDWaveData::metadataFound() const
{
  return data_metadata_found;
}


void RDWaveData::setMetadataFound(bool state)
{
  data_metadata_found=state;
}


unsigned RDWaveData::cartNumber() const
{
  return data_cart_number;
}


void RDWaveData::setCartNumber(unsigned cartnum)
{
  data_cart_number=cartnum;
}


int RDWaveData::cutNumber() const
{
  return data_cut_number;
}


void RDWaveData::setCutNumber(int cutnum)
{
  data_cut_number=cutnum;
}


QString RDWaveData::cutName() const
{
  return data_cutname;
}


void RDWaveData::setCutName(const QString &str)
{
  data_cutname=str;
}


RDWaveData::CartType RDWaveData::cartType() const
{
  return data_cart_type;
}


void RDWaveData::setCartType(RDWaveData::CartType type)
{
  data_cart_type=type;
}


QString RDWaveData::title() const
{
  return data_title;
}


void RDWaveData::setTitle(const QString &str)
{
  data_title=str;
}


QString RDWaveData::artist() const
{
  return data_artist;
}


void RDWaveData::setArtist(const QString &str)
{
  data_artist=str;
}


QString RDWaveData::album() const
{
  return data_album;
}


void RDWaveData::setAlbum(const QString &str)
{
  data_album=str;
}


QString RDWaveData::conductor() const
{
  return data_conductor;
}


void RDWaveData::setConductor(const QString &str)
{
  data_conductor=str;
}


QString RDWaveData::label() const
{
  return data_label;
}


void RDWaveData::setLabel(const QString &str)
{
  data_label=str;
}


int RDWaveData::releaseYear() const
{
  return data_release_year;
}


void RDWaveData::setReleaseYear(int year)
{
  data_release_year=year;
}


QString RDWaveData::client() const
{
  return data_client;
}


void RDWaveData::setClient(const QString &str)
{
  data_client=str;
}


QString RDWaveData::agency() const
{
  return data_agency;
}


void RDWaveData::setAgency(const QString &str)
{
  data_agency=str;
}


QString RDWaveData::composer() const
{
  return data_composer;
}


void RDWaveData::setComposer(const QString &str)
{
  data_composer=str;
}


QString RDWaveData::publisher() const
{
  return data_publisher;
}


void RDWaveData::setPublisher(const QString &str)
{
  data_publisher=str;
}


QString RDWaveData::songId() const
{
  return data_song_id;
}


void RDWaveData::setSongId(const QString &str)
{
  data_song_id=str;
}


RDWaveData::UsageCode RDWaveData::usageCode() const
{
  return data_usage_code;
}


void RDWaveData::setUsageCode(RDWaveData::UsageCode code)
{
  data_usage_code=code;
}


QStringList RDWaveData::schedCodes() const
{
  return data_sched_codes;
}


void RDWaveData::setSchedCodes(const QStringList &codes)
{
  data_sched_codes=codes;
}


QString RDWaveData::licensingOrganization() const
{
  return data_licensing_organization;
}


void RDWaveData::setLicensingOrganization(const QString &str)
{
  data_licensing_organization=str;
}


QString RDWaveData::copyrightNotice() const
{
  return data_copyright_notice;
}


void RDWaveData::setCopyrightNotice(const QString &str)
{
  data_copyright_notice=str;
}


QString RDWaveData::cutId() const
{
  return data_cut_id;
}


void RDWaveData::setCutId(const QString &str)
{
  data_cut_id=str;
}


QString RDWaveData::classification() const
{
  return data_classification;
}


void RDWaveData::setClassification(const QString &str)
{
  data_classification=str;
}


QString RDWaveData::category() const
{
  return data_category;
}


void RDWaveData::setCategory(const QString &str)
{
  data_category=str;
}


QString RDWaveData::url() const
{
  return data_url;
}


void RDWaveData::setUrl(const QString &str)
{
  data_url=str;
}


QString RDWaveData::tagText() const
{
  return data_tag_text;
}


void RDWaveData::setTagText(const QString &str)
{
  data_tag_text=str;
}


QString RDWaveData::description() const
{
  return data_description;
}


void RDWaveData::setDescription(const QString &str)
{
  data_description=str;
}


QString RDWaveData::originator() const
{
  return data_originator;
}


void RDWaveData::setOriginator(const QString &str)
{
  data_originator=str;
}


QString RDWaveData::originatorReference() const
{
  return data_originator_reference;
}


void RDWaveData::setOriginatorReference(const QString &str)
{
  data_originator_reference=str;
}


QString RDWaveData::codingHistory() const
{
  return data_coding_history;
}


void RDWaveData::setCodingHistory(const QString &str)
{
  data_coding_history=str;
}


QString RDWaveData::userDefined() const
{
  return data_user_defined;
}


void RDWaveData::setUserDefined(const QString &str)
{
  data_user_defined=str;
}


QString RDWaveData::isrc() const
{
  return data_isrc;
}


void RDWaveData::setIsrc(const QString &str)
{
  data_isrc=str;
}


QString RDWaveData::isci() const
{
  return data_isci;
}


void RDWaveData::setIsci(const QString &str)
{
  data_isci=str;
}


QString RDWaveData::recordingMbId() const
{
  return data_recording_mbid;
}


void RDWaveData::setRecordingMbId(const QString &mbid)
{
  data_recording_mbid=mbid;
}


QString RDWaveData::releaseMbId() const
{
  return data_release_mbid;
}


void RDWaveData::setReleaseMbId(const QString &mbid)
{
  data_release_mbid=mbid;
}


QString RDWaveData::mcn() const
{
  return data_mcn;
}


void RDWaveData::setMcn(const QString &str)
{
  data_mcn=str;
}


QString RDWaveData::outCue() const
{
  return data_out_cue;
}


void RDWaveData::setOutCue(const QString &str)
{
  data_out_cue=str;
}


RDWaveData::EndType RDWaveData::endType() const
{
  return data_end_type;
}


void RDWaveData::setEndType(RDWaveData::EndType type)
{
  data_end_type=type;
}


int RDWaveData::forcedLength() const
{
  return data_forced_length;
}


void RDWaveData::setForcedLength(int msecs)
{
  data_forced_length=msecs;
}


int RDWaveData::averageLength() const
{
  return data_average_length;
}


void RDWaveData::setAverageLength(int msecs)
{
  data_average_length=msecs;
}


int RDWaveData::lengthDeviation() const
{
  return data_length_deviation;
}


void RDWaveData::setLengthDeviation(int msecs)
{
  data_length_deviation=msecs;
}


int RDWaveData::minimumTalkLength() const
{
  return data_minimum_talk_length;
}


void RDWaveData::setMinimumTalkLength(int msecs)
{
  data_minimum_talk_length=msecs;
}


int RDWaveData::maximumTalkLength() const
{
  return data_maximum_talk_length;
}


void RDWaveData::setMaximumTalkLength(int msecs)
{
  data_maximum_talk_length=msecs;
}

int RDWaveData::averageSegueLength() const
{
  return data_average_segue_length;
}


void RDWaveData::setAverageSegueLength(int msecs)
{
  data_average_segue_length=msecs;
}


int RDWaveData::averageHookLength() const
{
  return data_average_hook_length;
}


void RDWaveData::setAverageHookLength(int msecs)
{
  data_average_hook_length=msecs;
}


int RDWaveData::cutQuantity() const
{
  return data_cut_quantity;
}


void RDWaveData::setCutQuantity(int n)
{
  data_cut_quantity=n;
}


int RDWaveData::lastCutPlayed() const
{
  return data_last_cut_played;
}


void RDWaveData::setLastCutPlayed(int cutnum)
{
  data_last_cut_played=cutnum;
}


QDateTime RDWaveData::lastPlayDatetime() const
{
  return data_last_play_datetime;
}


void RDWaveData::setLastPlayDatetime(const QDateTime &dt)
{
  data_last_play_datetime=dt;
}


int RDWaveData::length() const
{
  return data_length;
}


void RDWaveData::setLength(int msecs)
{
  data_length=msecs;
}


bool RDWaveData::enforceLength() const
{
  return data_enforce_length;
}


void RDWaveData::setEnforceLength(bool state)
{
  data_enforce_length=state;
}


bool RDWaveData::asyncronous() const
{
  return data_asyncronous;
}


void RDWaveData::setAsyncronous(bool state)
{
  data_asyncronous=state;
}


QString RDWaveData::owner() const
{
  return data_owner;
}


void RDWaveData::setOwner(const QString &str)
{
  data_owner=str;
}


QDateTime RDWaveData::metadataDatetime() const
{
  return data_metadata_datetime;
}


void RDWaveData::setMetadataDatetime(const QDateTime &dt)
{
  data_metadata_datetime=dt;
}


bool RDWaveData::dayOfWeek(int dow) const
{
  return data_day_of_week[dow-1];
}


void RDWaveData::setDayOfWeek(int dow,bool state)
{
  data_day_of_week[dow-1]=state;
}


int RDWaveData::weight() const
{
  return data_weight;
}


void RDWaveData::setWeight(int weight)
{
  data_weight=weight;
}


bool RDWaveData::evergreen() const
{
  return data_evergreen;
}


void RDWaveData::setEvergreen(bool state)
{
  data_evergreen=state;
}


int RDWaveData::talkStartPos() const
{
  return data_talk_start_pos;
}


void RDWaveData::setTalkStartPos(int msec)
{
  data_talk_start_pos=msec;
}


int RDWaveData::talkEndPos() const
{
  return data_talk_end_pos;
}


void RDWaveData::setTalkEndPos(int msec)
{
  data_talk_end_pos=msec;
}


int RDWaveData::segueStartPos() const
{
  return data_segue_start_pos;
}


void RDWaveData::setSegueStartPos(int msec)
{
  data_segue_start_pos=msec;
}


int RDWaveData::segueEndPos() const
{
  return data_segue_end_pos;
}


void RDWaveData::setSegueEndPos(int msec)
{
  data_segue_end_pos=msec;
}


int RDWaveData::segueGain() const
{
  return data_segue_gain;
}


void RDWaveData::setSegueGain(int lvl)
{
  data_segue_gain=lvl;
}


int RDWaveData::startPos() const
{
  return data_start_pos;
}


void RDWaveData::setStartPos(int msec)
{
  data_start_pos=msec;
}


int RDWaveData::endPos() const
{
  return data_end_pos;
}


void RDWaveData::setEndPos(int msec)
{
  data_end_pos=msec;
}


int RDWaveData::hookStartPos() const
{
  return data_hook_start_pos;
}


void RDWaveData::setHookStartPos(int msec)
{
  data_hook_start_pos=msec;
}


int RDWaveData::hookEndPos() const
{
  return data_hook_end_pos;
}


void RDWaveData::setHookEndPos(int msec)
{
  data_hook_end_pos=msec;
}


int RDWaveData::fadeUpPos() const
{
  return data_fade_up_pos;
}


void RDWaveData::setFadeUpPos(int msec)
{
  data_fade_up_pos=msec;
}


int RDWaveData::fadeDownPos() const
{
  return data_fade_down_pos;
}


void RDWaveData::setFadeDownPos(int msec)
{
  data_fade_down_pos=msec;
}


int RDWaveData::beatsPerMinute() const
{
  return data_beats_per_minute;
}


void RDWaveData::setBeatsPerMinute(int bpm)
{
  data_beats_per_minute=bpm;
}


QString RDWaveData::tmciSongId() const
{
  return data_tmci_song_id;
}


void RDWaveData::setTmciSongId(const QString &str)
{
  data_tmci_song_id=str;
}


QDate RDWaveData::startDate() const
{
  return data_start_date;
}


void RDWaveData::setStartDate(const QDate &date)
{
  data_start_date=date;
  data_datetime_set=true;
}


QTime RDWaveData::startTime() const
{
  return data_start_time;
}


void RDWaveData::setStartTime(const QTime &time)
{
  data_start_time=time;
  data_datetime_set=true;
}


QDateTime RDWaveData::startDateTime() const
{
  return QDateTime(data_start_date,data_start_time);
}


QDate RDWaveData::originationDate() const
{
  return data_origination_date;
}


void RDWaveData::setOriginationDate(const QDate &date)
{
  data_origination_date=date;
}


QTime RDWaveData::originationTime() const
{
  return data_origination_time;
}


void RDWaveData::setOriginationTime(const QTime &time)
{
  data_origination_time=time;
}


QDate RDWaveData::endDate() const
{
  return data_end_date;
}


void RDWaveData::setEndDate(const QDate &date)
{
  data_end_date=date;
  data_datetime_set=true;
}


QTime RDWaveData::endTime() const
{
  return data_end_time;
}


void RDWaveData::setEndTime(const QTime &time)
{
  data_end_time=time;
  data_datetime_set=true;
}


QDateTime RDWaveData::endDateTime() const
{
  return QDateTime(data_end_date,data_end_time);
}


QTime RDWaveData::daypartStartTime() const
{
  return data_daypart_start_time;
}


void RDWaveData::setDaypartStartTime(const QTime &time)
{
  data_daypart_start_time=time;
}


QTime RDWaveData::daypartEndTime() const
{
  return data_daypart_end_time;
}


void RDWaveData::setDaypartEndTime(const QTime &time)
{
  data_daypart_end_time=time;
}


unsigned RDWaveData::playCounter() const
{
  return data_play_counter;
}


void RDWaveData::setPlayCounter(unsigned count)
{
  data_play_counter=count;
}


RDSettings RDWaveData::audioSettings() const
{
  return data_settings;
}


void RDWaveData::setAudioSettings(const RDSettings &settings)
{
  data_settings=settings;
}


int RDWaveData::playGain() const
{
  return data_play_gain;
}


void RDWaveData::setPlayGain(int lvl)
{
  data_play_gain=lvl;
}


bool RDWaveData::validateMarkers(int msec)
{
  int len=msec;
  bool modified=false;

  if(len<0) {
    len=length();
  }

  //
  // Start/End Markers
  //
  if(startPos()<0) {
    setStartPos(0);
    modified=true;
  }
  if((endPos()<0)||(endPos()>len)) {
    setEndPos(len);
    modified=true;
  }
  if(startPos()>endPos()) {
    setStartPos(0);
    setEndPos(len);
    modified=true;
  }

  //
  // Talk Markers
  //
  if((talkStartPos()==startPos())&&(talkEndPos()==endPos())) {
    setTalkStartPos(-1);
    setTalkEndPos(-1);
    modified=true;
  }
  if((talkStartPos()<0)||(talkEndPos()<0)||(talkStartPos()>talkEndPos())) {
    setTalkStartPos(-1);
    setTalkEndPos(-1);
    modified=true;
  }
  else {
    if(talkEndPos()>endPos()) {
      setTalkEndPos(endPos());
      modified=true;
    }
    if(talkStartPos()>endPos()) {
      setTalkStartPos(-1);
      setTalkEndPos(-1);
      modified=true;
    }
  }

  //
  // Segue Markers
  //
  if((segueStartPos()==startPos())&&(segueEndPos()==endPos())) {
    setSegueStartPos(-1);
    setSegueEndPos(-1);
    modified=true;
  }
  if((segueStartPos()<0)||(segueEndPos()<0)||(segueStartPos()>segueEndPos())) {
    setSegueStartPos(-1);
    setSegueEndPos(-1);
    modified=true;
  }
  else {
    if(segueEndPos()>endPos()) {
      setSegueEndPos(endPos());
      modified=true;
    }
    if(segueStartPos()>endPos()) {
      setSegueStartPos(-1);
      setSegueEndPos(-1);
      modified=true;
    }
  }

  //
  // Hook Markers
  //
  if((hookStartPos()<0)||(hookEndPos()<0)||(hookStartPos()>hookEndPos())) {
    setHookStartPos(-1);
    setHookEndPos(-1);
    modified=true;
  }
  else {
    if(hookEndPos()>endPos()) {
      setHookEndPos(endPos());
      modified=true;
    }
    if(hookStartPos()>endPos()) {
      setHookStartPos(-1);
      setHookEndPos(-1);
      modified=true;
    }
  }

  //
  // FadeUp Marker
  //
  if(fadeUpPos()>=0) {
    if(fadeUpPos()>endPos()) {
      setFadeUpPos(endPos());
      modified=true;
    }
  }

  //
  // FadeDown Marker
  //
  if(fadeDownPos()>=0) {
    if(fadeDownPos()>endPos()) {
      setFadeDownPos(endPos());
      modified=true;
    }
  }

  return modified;
}


bool RDWaveData::validateDateTimes()
{
  if(data_datetime_set) {
    if(startDateTime()<endDateTime()) {
      return false;
    }
    else {
      setStartDate(QDate());
      setEndDate(QDate());
      setStartTime(QTime());
      setEndTime(QTime());
      return true;
    }
  }
  return false;
}


QString RDWaveData::dump() const
{
  QString ret="";

  //
  // Cart-level attributes
  //
  ret+=QString::asprintf("cartNumber: %06u\n",cartNumber());
  ret+="cartType: "+RDWaveData::cartTypeText(cartType())+"\n";
  ret+="category: "+category()+"\n";
  ret+="title: "+title()+"\n";
  ret+="artist: "+artist()+"\n";
  ret+="album: "+album()+"\n";
  ret+="label: "+label()+"\n";
  ret+="agency: "+agency()+"\n";
  ret+="client: "+client()+"\n";
  ret+="composer: "+composer()+"\n";
  ret+="publisher: "+publisher()+"\n";
  ret+="conductor: "+conductor()+"\n";
  ret+="userDefined: "+userDefined()+"\n";
  if(releaseYear()>0) {
    ret+=QString::asprintf("year: %d\n",releaseYear());
  }
  else {
    ret+="year:\n";
  }
  ret+="usageCode: "+RDWaveData::usageText(usageCode())+"\n";
  ret+="forcedLength: "+RDGetTimeLength(forcedLength(),true,true)+"\n";
  ret+="averageLength: "+RDGetTimeLength(averageLength(),true,true)+"\n";
  ret+="lengthDeviation: "+RDGetTimeLength(lengthDeviation(),true,true)+"\n";
  ret+=
    "averageSegueLength: "+RDGetTimeLength(averageSegueLength(),true,true)+"\n";
  ret+="averageHookLength: "+RDGetTimeLength(averageHookLength(),true,true)+"\n";
  ret+=QString::asprintf("cutQuantity: %d\n",cutQuantity());
  ret+=QString::asprintf("lastCutPlayed: %d\n",lastCutPlayed());
  if(enforceLength()) {
    ret+="enforceLength: true\n";
  }
  else {
    ret+="enforceLength: false\n";
  }
  if(asyncronous()) {
    ret+="asyncronous: true\n";
  }
  else {
    ret+="asyncronous: false\n";
  }
  ret+="owner: "+owner()+"\n";
  ret+="metadataDatetime: "+
    metadataDatetime().toString("yyyy-MM-dd hh:mm:ss")+"\n";

  //
  // Cut-level attributes
  //
  ret+="cutName: "+cutName()+"\n";
  ret+=QString::asprintf("cutNumber: %d\n",cutNumber());
  ret+="description: "+description()+"\n";
  ret+="outcue: "+outCue()+"\n";
  if(evergreen()) {
    ret+="evergreen: true\n";
  }
  else {
    ret+="evergreen: false\n";
  }
  ret+="isrc: "+isrc()+"\n";
  ret+="isci: "+isci()+"\n";
  ret+="recordingMbId: "+recordingMbId()+"\n";
  ret+="releaseMbId: "+releaseMbId()+"\n";
  ret+=QString::asprintf("length: %d\n",length());
  ret+="originator: "+originator()+"\n";
  ret+="originationDate: "+originationDate().toString("yyyy-MM-dd")+"\n";
  ret+="originationTime: "+originationTime().toString("hh:mm:ss")+"\n";
  ret+="startDate: ";
  if(startDate().isValid()) {
    ret+=startDate().toString("yyyy-MM-dd");
  }
  ret+="\n";
  ret+="startTime: ";
  if(startDate().isValid()&&startTime().isValid()) {
    ret+=startTime().toString("hh:mm:ss");
  }
  ret+="\n";
  ret+="endDate: ";
  if(endDate().isValid()) {
    ret+=endDate().toString("yyyy-MM-dd");
  }
  ret+="\n";
  ret+="endTime: ";
  if(endDate().isValid()&&endTime().isValid()) {
    ret+=endTime().toString("hh:mm:ss");
  }
  ret+="\n";
  for(int i=1;i<8;i++) {
    if(dayOfWeek(i)) {
      ret+=QDate::longDayName(i)+": true\n";
    }
    else {
      ret+=QDate::longDayName(i)+": false\n";
    }
  }
  ret+="daypartStartTime: ";
  if(daypartStartTime().isValid()&&(daypartStartTime()!=daypartEndTime())) {
    ret+=daypartStartTime().toString("hh:mm:ss");
  }
  ret+="\n";
  ret+="daypartEndTime: ";
  if(daypartEndTime().isValid()&&(daypartStartTime()!=daypartEndTime())) {
    ret+=daypartEndTime().toString("hh:mm:ss");
  }
  ret+="\n";
  ret+="lastPlayDatetime: ";
  if(lastPlayDatetime().isValid()) {
    ret+=lastPlayDatetime().toString("yyyy-MM-dd hh:mm:ss");
  }
  ret+="\n";
  ret+=QString::asprintf("weight: %d\n",weight());
  ret+=QString::asprintf("playCounter: %d\n",playCounter());
  ret+=QString::asprintf("audioSettings::format: %u\n",
			 audioSettings().format());
  ret+=QString::asprintf("audioSettings::sampleRate: %u\n",
			 audioSettings().sampleRate());
  ret+=QString::asprintf("audioSettings::bitRate: %u\n",
			 audioSettings().bitRate());
  ret+=QString::asprintf("audioSettings::channels: %u\n",
			 audioSettings().channels());
  ret+=QString::asprintf("playGain: %d\n",playGain());
  ret+=QString::asprintf("startPos: %d\n",startPos());
  ret+=QString::asprintf("endPos: %d\n",endPos());
  ret+=QString::asprintf("segueStartPos: %d\n",segueStartPos());
  ret+=QString::asprintf("segueEndPos: %d\n",segueEndPos());
  ret+=QString::asprintf("segueGain: %d\n",segueGain());
  ret+=QString::asprintf("talkStartPos: %d\n",talkStartPos());
  ret+=QString::asprintf("talkEndPos: %d\n",talkEndPos());
  ret+=QString::asprintf("hookStartPos: %d\n",hookStartPos());
  ret+=QString::asprintf("hookEndPos: %d\n",hookEndPos());
  ret+=QString::asprintf("fadeUpPos: %d\n",fadeUpPos());
  ret+=QString::asprintf("fadeDownPos: %d\n",fadeDownPos());
  ret+="\n";

  return ret;
}

void RDWaveData::clear()
{
  data_metadata_found=false;
  data_datetime_set=false;
  data_cart_number=0;
  data_cart_type=RDWaveData::AudioType;
  data_cut_number=0;
  data_cutname="";
  data_title="";
  data_artist="";
  data_album="";
  data_conductor="";
  data_label="";
  data_release_year=0;
  data_client="";
  data_agency="";
  data_composer="";
  data_publisher="";
  data_usage_code=RDWaveData::UsageFeature;
  data_sched_codes.clear();
  data_licensing_organization="";
  data_copyright_notice="";
  data_user_defined="";
  data_cut_id="";
  data_classification="";
  data_category="";
  data_url="";
  data_tag_text="";
  data_description="";
  data_originator="";
  data_originator_reference="";
  data_isrc="";
  data_isci="";
  data_recording_mbid="";
  data_release_mbid="";
  data_mcn="";
  data_out_cue="";
  data_release_year=0;
  data_end_type=RDWaveData::UnknownEnd;

  data_length=0;
  data_forced_length=-1;
  data_average_length=-1;
  data_length_deviation=0;
  data_average_segue_length=0;
  data_average_hook_length=0;
  data_cut_quantity=0;
  data_last_cut_played=0;
  data_last_play_datetime=QDateTime();
  data_enforce_length=false;
  data_asyncronous=false;
  data_owner="";
  data_metadata_datetime=QDateTime();

  for(int i=0;i<7;i++) {
    data_day_of_week[i]=true;
  }
  data_weight=1;
  data_evergreen=false;
  data_talk_start_pos=-1;
  data_talk_end_pos=-1;
  data_segue_start_pos=-1;
  data_segue_end_pos=-1;
  data_segue_gain=-3000;
  data_start_pos=-1;
  data_end_pos=-1;
  data_hook_start_pos=-1;
  data_hook_end_pos=-1;
  data_fade_up_pos=-1;
  data_fade_down_pos=-1;
  data_beats_per_minute=0;
  data_tmci_song_id="";
  data_origination_date=QDate();
  data_origination_time=QTime();
  data_start_date=QDate();
  data_start_time=QTime();
  data_end_date=QDate();
  data_end_time=QTime();
  data_daypart_start_time=QTime();
  data_daypart_end_time=QTime();
  data_play_counter=0;
  data_play_gain=0;
}


QString RDWaveData::endTypeText(EndType type)
{
  QString ret=QObject::tr("unknown");

  switch(type) {
  case UnknownEnd:
    ret=QObject::tr("unknown");
    break;

  case ColdEnd:
    ret=QObject::tr("cold");
    break;

  case FadeEnd:
    ret=QObject::tr("fade");
    break;
  }

  return ret;
}


QString RDWaveData::cartTypeText(CartType type)
{
  QString ret=QObject::tr("unknown");

  switch(type) {
  case RDWaveData::AudioType:
    ret="audio";
    break;

  case RDWaveData::MacroType:
    ret="macro";
    break;
  }

  return ret;
}


QString RDWaveData::usageText(UsageCode code)
{
  QString ret=QObject::tr("unknown");

  switch(code) {
  case UsageFeature:
    ret=QObject::tr("feature");
    break;

  case UsageOpen:
    ret=QObject::tr("open");
    break;

  case UsageClose:
    ret=QObject::tr("close");
    break;

  case UsageTheme:
    ret=QObject::tr("theme");
    break;

  case UsageBackground:
    ret=QObject::tr("background");
    break;

  case UsagePromo:
    ret=QObject::tr("promo");
    break;

  case UsageLast:
    break;
  }

  return ret;
}


QString RDWaveData::dump(const QString &label,RDWaveData *data)
{
  QString ret=label+": ";

  if(data==NULL) {
    ret+="[NULL]";
  }
  else {
    ret+=data->dump();
  }

  return ret;
}
