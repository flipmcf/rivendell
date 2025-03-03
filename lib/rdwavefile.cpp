//   rdwavefile.cpp
//
//   A class for handling audio files.
//
//   (C) Copyright 2002-2021 Fred Gleason <fredg@paravelsystems.com>
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
//

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <math.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <arpa/inet.h>

#include <typeinfo>

#include <id3/tag.h>
#include <id3/misc_support.h>
#include <fileref.h>
#include <apetag.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <tag.h>
#include <tpropertymap.h>
#ifdef HAVE_FLAC
#include <FLAC/metadata.h>
#endif  // HAVE_FLAC

#include <rd.h>
#include <rdcart.h>
#include <rdwavefile.h>
#include <rdconf.h>
#include <rdmp4.h>

#ifdef HAVE_MP4_LIBS
#include <mp4v2/mp4v2.h>
#endif

RDWaveFile::RDWaveFile(QString file_name)
{
  // 
  // Initialize Class Structures
  //
  wave_file_name=file_name;
  wave_file.setFileName(file_name);
  wave_data=NULL;
  recordable=false;
  format_chunk=false;
  comm_chunk=false;
  format_tag=0;
  channels=0;
  normalize_level=1.0;
  samples_per_sec=0;
  avg_bytes_per_sec=0;
  block_align=0;
  bits_per_sample=0;
  cb_size=0;
  head_layer=0;
  head_bit_rate=0;
  head_mode=0;
  head_mode_ext=0;
  head_emphasis=1;
  head_flags=0;
  mpeg_frame_size=0;
  id3v1_tag=false;
  id3v2_tag[0]=false;
  id3v2_tag[1]=false;
  id3v2_offset[0]=0;
  id3v2_offset[1]=0;
  has_energy=false;
  energy_loaded=false;
  energy_ptr=0;
  for(int i=0;i<FMT_CHUNK_SIZE;i++) {
    fmt_chunk_data[i]=0;
  }
  fmt_size=0;
  fact_chunk=false;
  sample_length=0;
  for(int i=0;i<FACT_CHUNK_SIZE;i++) {
    fact_chunk_data[i]=0;
  }
  data_chunk=false;
  data_length=0;
  cart_chunk=false;
  cart_version=0;
  cart_title="";
  cart_title="";
  cart_artist="";
  cart_cut_id="";
  cart_client_id="";
  cart_category="";
  cart_classification="";
  cart_out_cue="";
  cart_producer_app_id="";
  cart_producer_app_ver="";
  cart_user_def="";
  cart_url="";
  cart_tag_text="";
  cart_level_ref=CART_DEFAULT_LEVEL_REF;
  for(int i=0;i<MAX_TIMERS;i++) {
    cart_timer_label[i]=QString("");
    cart_timer_sample[i]=0;
  }
  for(int i=0;i<CART_CHUNK_SIZE;i++) {
    cart_chunk_data[i]=0;
  }
  bext_chunk=false;
  bext_description="";
  bext_originator="";
  bext_originator_ref="";
  bext_origination_date=QDate::currentDate();
  bext_origination_time=QTime::currentTime();
  bext_time_reference_low=0;
  bext_time_reference_high=0;
  bext_version=0;
  for(int i=0;i<64;i++) {
    bext_umid[i]=0;
  }
  bext_coding_history="";
  for(int i=0;i<BEXT_CHUNK_SIZE;i++) {
    bext_chunk_data[i]=0;
  }
  bext_coding_data=NULL;
  bext_coding_size=0;
  mext_chunk=false;
  mext_homogenous=true;
  mext_padding_used=false;
  mext_rate_hacked=false;
  mext_free_format=false;
  mext_frame_size=0;
  mext_anc_length=0;
  mext_left_energy=false;
  mext_ancillary_private=false;
  mext_right_energy=false;
  for(int i=0;i<MEXT_CHUNK_SIZE;i++) {
    mext_chunk_data[i]=0;
  }
  levl_chunk=false;
  levl_format=DEFAULT_LEVL_FORMAT; 
  levl_points=DEFAULT_LEVL_POINTS;
  levl_block_size=DEFAULT_LEVL_BLOCK_SIZE;
  cook_buffer=NULL;
  cook_buffer_size=0;
  wave_type=RDWaveFile::Unknown;
  encode_quality=5.0f;
  serial_number=-1;
  atx_offset=0;
  scot_chunk=false;
  av10_chunk=false;
  rdxl_chunk=false;
  ptr_offset_msecs=0;
}


RDWaveFile::~RDWaveFile()
{
  if(bext_coding_data!=NULL) {
    free(bext_coding_data);
  }
  if(cook_buffer!=NULL) {
    free(cook_buffer);
  }
}


RDWaveFile::Type RDWaveFile::type() const
{
  return wave_type;
}


void RDWaveFile::nameWave(QString file_name)
{
  if(wave_file.isOpen()) {
    return;
  }
  wave_file.setFileName(file_name);
  wave_file_name=file_name;
}



bool RDWaveFile::openWave(RDWaveData *data)
{
#ifdef HAVE_VORBIS
  vorbis_info *vorbis_info;
#endif  // HAVE_VORBIS
  unsigned char tmc_buffer[4];
  int fd=-1;

  wave_data=data;
  if((fd=open(wave_file_name.toUtf8(),O_RDONLY))<0) {
    return false;
  }
  if(!wave_file.open(fd,QIODevice::ReadOnly,QFile::AutoCloseHandle)) {
    return false;
  }
  switch(GetType(wave_file.handle())) {
  case RDWaveFile::Wave:
    if(GetFmt(wave_file.handle())) {
      wave_type=RDWaveFile::Wave;
    }
    else {
      wave_type=RDWaveFile::Ambos;
      format_tag=WAVE_FORMAT_MPEG;
    }
    if(!GetChunk(wave_file.handle(),"data",&data_length,NULL,0)) { // Set fileptr to start of data block
      return false;
    }
    data_chunk=true;
    data_start=lseek(wave_file.handle(),0,SEEK_CUR);
    if((!GetFact(wave_file.handle()))||(sample_length==0)) {
      if((format_tag!=WAVE_FORMAT_PCM)&&
	 (format_tag!=WAVE_FORMAT_IEEE_FLOAT)&&format_chunk) {
#ifdef MPEG_FACT_FUDGE
	// Guesstimate the overall sample size
	sample_length=1152.0*((double)data_length/(144.0*(double)head_bit_rate/
						   (double)samples_per_sec));
	ext_time_length=1000.0*(double)sample_length/(double)samples_per_sec;
	time_length=ext_time_length/1000;
#else
	time_length=0;
	sample_length=0;
	ext_time_length=0;
#endif
      }
      else {
	if(format_chunk) {
	  ext_time_length=(unsigned)(1000.0*(double)data_length/
				     (double)(block_align*samples_per_sec));
	  time_length=ext_time_length/1000;
	  sample_length=data_length/block_align;
	}
	else {
	  if(!GetMpegHeader(wave_file.handle(),data_start)) {
	    wave_file.close();
	    return false;
	  }
	  data_length=wave_file.size()-data_start;
	  sample_length=1152*(data_length/mpeg_frame_size);
	  lseek(wave_file.handle(),data_start,SEEK_SET);
	  format_chunk=true;
	}
      }
    }
    else {
      if(format_chunk) {
	time_length=sample_length/samples_per_sec;
	ext_time_length=(unsigned)(1000.0*(double)sample_length/
				   (double)samples_per_sec);
      }
      else {
	time_length=0;
	ext_time_length=0;
      }
    }
    GetCart(wave_file.handle());
    GetBext(wave_file.handle());
    GetMext(wave_file.handle());
    GetList(wave_file.handle());
    GetScot(wave_file.handle());
    GetAv10(wave_file.handle());
    GetAir1(wave_file.handle());
    GetRdxl(wave_file.handle());
    break;

  case RDWaveFile::Aiff:
    if(GetComm(wave_file.handle())) {
      wave_type=RDWaveFile::Aiff;
    }
    if(!GetChunk(wave_file.handle(),"SSND",&data_length,NULL,0,true)) {
      return false;
    }
    data_length-=8;  // SSND chunk has eight data bytes at the beginning!
    data_chunk=true;
    data_start=lseek(wave_file.handle(),8,SEEK_CUR);
    ext_time_length=(unsigned)(1000.0*(double)sample_length/
			       (double)samples_per_sec);
    time_length=ext_time_length/1000;
    break;

  case RDWaveFile::Mpeg:
    format_tag=WAVE_FORMAT_MPEG;
    if(!GetMpegHeader(wave_file.handle(),id3v2_offset[0])) {
      wave_file.close();
      return false;
    }
    data_length=wave_file.size();
    if(id3v1_tag) {
      data_length-=128;
    }
    if(id3v2_tag[1]) {
      data_length-=id3v2_offset[1];
    }
    data_start=id3v2_offset[0];
    sample_length=1152*(data_length/mpeg_frame_size);
    ext_time_length=1000*((uint64_t)sample_length)/((uint64_t)samples_per_sec);
    data_chunk=true;
    lseek(wave_file.handle(),data_start,SEEK_SET);
    format_chunk=true;
    wave_type=RDWaveFile::Mpeg;
    ReadId3Metadata();
    break;

  case RDWaveFile::M4A:
    {
#ifdef HAVE_MP4_LIBS   

      // MP4 libs must already be loaded by now for file to have that type.
      assert(dlmp4.load());
      format_tag=WAVE_FORMAT_M4A;

      MP4FileHandle f = dlmp4.MP4Read(getName().toUtf8());
      if(f == MP4_INVALID_FILE_HANDLE)
	return false;

      // Find an audio track, and populate sample rate, bits/sample etc.
      MP4TrackId audioTrack = dlmp4.getMP4AACTrack(f);

      if(audioTrack == MP4_INVALID_TRACK_ID) {
	dlmp4.MP4Close(f, 0);
	return false;
      }

      // Found audio track. Get audio data:
      avg_bytes_per_sec = dlmp4.MP4GetTrackBitRate(f, audioTrack);
      channels = dlmp4.MP4GetTrackAudioChannels(f, audioTrack);

      MP4Duration trackDuration = dlmp4.MP4GetTrackDuration(f, audioTrack);
      ext_time_length = (unsigned)dlmp4.MP4ConvertFromTrackDuration(f, audioTrack, trackDuration, 
								    MP4_MSECS_TIME_SCALE);
      time_length = ext_time_length / 1000;
      samples_per_sec = dlmp4.MP4GetTrackTimeScale(f, audioTrack);
      bits_per_sample = 16;
      data_start = 0;
      sample_length = dlmp4.MP4GetTrackNumberOfSamples(f, audioTrack);
      data_length = sample_length * 2 * channels;
      data_chunk = true;
      format_chunk = true;
      wave_type = RDWaveFile::M4A;

      // Now extract metadata (title, artist, etc)

      if(wave_data) {

	const MP4Tags* tags = dlmp4.MP4TagsAlloc();
	dlmp4.MP4TagsFetch(tags, f);
	
	wave_data->setMetadataFound(true);
	
	if(tags->name) {
	  wave_data->setTitle(tags->name);
	}
	if(tags->artist) {
	  wave_data->setArtist(tags->artist);
	}
	if(tags->composer) {
	  wave_data->setComposer(tags->composer);
	}
	if(tags->album) {
	  wave_data->setAlbum(tags->album);
	}

	dlmp4.MP4TagsFree(tags);
	wave_data->validateMarkers(ext_time_length);
      }

      dlmp4.MP4Close(f, 0);
      return true;

#else
      return false;
#endif
      break;
    }

  case RDWaveFile::Ogg:
#ifdef HAVE_VORBIS
    format_tag=WAVE_FORMAT_VORBIS;
    avg_bytes_per_sec=ov_bitrate(&vorbis_file,-1)/8;
    vorbis_info=ov_info(&vorbis_file,-1);
    channels=vorbis_info->channels;
    block_align=2*channels;
    samples_per_sec=vorbis_info->rate;
    avg_bytes_per_sec=block_align*samples_per_sec;
    bits_per_sample=16;
    data_start=0;
    sample_length=ov_pcm_total(&vorbis_file,-1);
    data_length=sample_length*2*channels;
    ext_time_length=(unsigned)(ov_time_total(&vorbis_file,-1)*1000.0);
    time_length=(unsigned)ov_time_total(&vorbis_file,-1);
    data_chunk=true;
    format_chunk=true;
    wave_type=RDWaveFile::Ogg;
    ReadNormalizeLevel(wave_file_name);
    return true;
#else
    return false;
#endif  // HAVE_VORBIS
    break;

  case RDWaveFile::Atx:
    format_tag=WAVE_FORMAT_MPEG;
    atx_offset=GetAtxOffset(wave_file.handle());
    if(!GetMpegHeader(wave_file.handle(),atx_offset)) {
      wave_file.close();
      return false;
    }
    data_length=wave_file.size()-atx_offset;
    data_start=atx_offset;
    sample_length=1152*(data_length/mpeg_frame_size);
    ext_time_length=
      (unsigned)(1000.0*(double)sample_length/(double)samples_per_sec);
    time_length=ext_time_length/1000;
    data_chunk=true;
    lseek(wave_file.handle(),data_start,SEEK_SET);
    format_chunk=true;
    wave_type=RDWaveFile::Atx;	
    break;

  case RDWaveFile::Tmc:
    format_tag=WAVE_FORMAT_MPEG;
    atx_offset=4;
    if(!GetMpegHeader(wave_file.handle(),atx_offset)) {
      wave_file.close();
      return false;
    }
    lseek(wave_file.handle(),0,SEEK_SET);
    CheckExitCode("RDWaveFile::openWave()",
		  read(wave_file.handle(),tmc_buffer,4));
    data_length=(0xFF&tmc_buffer[0])+(0xFF&tmc_buffer[1])*256+
      (0xFF&tmc_buffer[2])*65536+(0xFF&tmc_buffer[3])*16777216;
    data_start=atx_offset;
    sample_length=1152*(data_length/mpeg_frame_size);
    ext_time_length=
      (unsigned)(1000.0*(double)sample_length/(double)samples_per_sec);
    time_length=ext_time_length/1000;
    data_chunk=true;
    lseek(wave_file.handle(),data_start,SEEK_SET);
    format_chunk=true;
    wave_type=RDWaveFile::Tmc;
    ReadTmcMetadata(wave_file.handle());
    break;
#ifdef HAVE_FLAC
  case RDWaveFile::Flac:
    format_tag=WAVE_FORMAT_FLAC;
    if(!GetFlacStreamInfo()) {
      wave_file.close();
      return false;
    }
    wave_type=RDWaveFile::Flac;
    format_chunk=true;
    if(wave_data!=NULL) {
      ReadId3Metadata();
      ReadFlacMetadata();
    }
    break;
#endif  // HAVE_FLAC

  default:
    close(wave_file.handle());
    return false;
    break;
  }
  lseek(wave_file.handle(),data_start,SEEK_SET);

  if(wave_data) {
    wave_data->validateMarkers(ext_time_length);
  }

  return true;
}


bool RDWaveFile::createWave(RDWaveData *data,unsigned ptr_offset)
{
  mode_t prev_mask;
  bool rc;
  wave_data=data;
  ptr_offset_msecs=ptr_offset;
  if(wave_data!=NULL) {
    cart_title=wave_data->title();
    cart_artist=wave_data->artist();
    cart_cut_id=wave_data->cutId();
    cart_client_id=wave_data->client();
    cart_category=wave_data->category();
    cart_classification=wave_data->classification();
    cart_out_cue=wave_data->outCue();
    cart_start_date=wave_data->startDate();
    cart_start_time=wave_data->startTime();
    cart_end_date=wave_data->endDate();
    cart_end_time=wave_data->endTime();
    cart_user_def=wave_data->userDefined();
    cart_url=wave_data->url();
    cart_tag_text=wave_data->tagText();
    bext_description=wave_data->description();
    bext_originator=wave_data->originator();
    bext_originator_ref=wave_data->originatorReference();
    bext_coding_history=wave_data->codingHistory();
  }

  switch(format_tag) {
      case WAVE_FORMAT_PCM:
      case WAVE_FORMAT_MPEG:
	levl_block_ptr=0;
	levl_istate=0;
	levl_accum=0;
	energy_data.clear();
	for(int i=0;i<channels;i++) {
	  energy_data.push_back(0);
	}
	if(!MakeFmt()) {
	  return false;
	}
        prev_mask = umask(0113);      // Set umask so files are user and group writable.
        rc=wave_file.open(QIODevice::ReadWrite|QIODevice::Truncate);
	unlink((wave_file_name+".energy").toUtf8());
        umask(prev_mask);
	if(rc==false) {
	  return false;
	}
	recordable=true;
	CheckExitCode("RDWaveFile::createWave()",
		      write(wave_file.handle(),"RIFF\0\0\0\0WAVE",12));
	WriteChunk(wave_file.handle(),"fmt ",fmt_chunk_data,fmt_size);
	if(format_tag==WAVE_FORMAT_MPEG) {
	  CheckExitCode("RDWaveFile::createWave()",
			write(wave_file.handle(),"fact\4\0\0\0\0\0\0\0",12));
	}
	if(cart_chunk) {
	  MakeCart(ptr_offset);
	  WriteChunk(wave_file.handle(),"cart",cart_chunk_data,
		     CART_CHUNK_SIZE);
	}
	if(bext_chunk) {
	  MakeBext();
	  WriteChunk(wave_file.handle(),"bext",bext_coding_data,
		     bext_coding_size);
	}
	if(mext_chunk) {
	  MakeMext();
	  WriteChunk(wave_file.handle(),"mext",mext_chunk_data,
		     MEXT_CHUNK_SIZE);
	}
	if(!rdxl_contents.isEmpty()) {
	  WriteChunk(wave_file.handle(),"rdxl",rdxl_contents);
	}
	wave_type=RDWaveFile::Wave;
	CheckExitCode("RDWaveFile::createWave()",
		      write(wave_file.handle(),"data\0\0\0\0",8));
	data_start=lseek(wave_file.handle(),0,SEEK_CUR);
	break;

      case WAVE_FORMAT_VORBIS:
#ifdef HAVE_VORBIS
	avg_bytes_per_sec=2*channels*samples_per_sec;
	vorbis_info_init(&vorbis_inf);
	if(vorbis_encode_init_vbr(&vorbis_inf,channels,samples_per_sec,
				  encode_quality)<0) {
	  vorbis_info_clear(&vorbis_inf);
	  return false;
	}
	vorbis_encode_ctl(&vorbis_inf,OV_ECTL_RATEMANAGE_SET,NULL);

        prev_mask = umask(0113);      // Set umask so files are user and group writable.
	    rc=wave_file.open(QIODevice::ReadWrite|QIODevice::Truncate);
        umask(prev_mask);
	if(rc==false) {
	  vorbis_info_clear(&vorbis_inf);
	  return false;
	}
	wave_type=RDWaveFile::Ogg;
	recordable=true;
	vorbis_encode_setup_init(&vorbis_inf);
	vorbis_analysis_init(&vorbis_dsp,&vorbis_inf);
	vorbis_block_init(&vorbis_dsp,&vorbis_blk);
	if(serial_number<0) {  // Generate random serial number
	  srand(time(NULL));
	  serial_number=abs(rand());
	}
	ogg_stream_init(&ogg_stream,serial_number);
	{
	  ogg_packet header_main;
	  ogg_packet header_comments;
	  ogg_packet header_codebooks;
	  vorbis_comment vorbis_comm;
	  vorbis_comment_init(&vorbis_comm);
	  
	  vorbis_analysis_headerout(&vorbis_dsp,&vorbis_comm,&header_main,
				    &header_comments,&header_codebooks);
	  ogg_stream_packetin(&ogg_stream,&header_main);
	  ogg_stream_packetin(&ogg_stream,&header_comments);
	  ogg_stream_packetin(&ogg_stream,&header_codebooks);
	  while(ogg_stream_flush(&ogg_stream,&ogg_pg)) {
	    WriteOggPage(&ogg_pg);
	  }
	}
	return true;
#endif  // HAVE_VORBIS
	return false;
	break;
  }
  levl_timestamp=QDateTime(QDate::currentDate(),QTime::currentTime());
  data_length=0;
  return true;
}


void RDWaveFile::closeWave(int samples)
{
  unsigned char size_buf[4];
  unsigned csize;
  unsigned lsize=0;
  unsigned cptr;

  if(recordable) {
    switch(wave_type) {
	case RDWaveFile::Wave:
	  //
	  // Write levl chunk
	  //
	  if(levl_chunk&&((format_tag==WAVE_FORMAT_PCM)||
			  ((format_tag==WAVE_FORMAT_MPEG)&&(head_layer==2)))) {
	    levl_version=0;
	    levl_format=2;
	    levl_points=1;
	    levl_block_size=1152;
	    levl_channels=channels;
	    levl_frames=energy_data.size()/channels;
	    levl_peak_offset=0xFFFFFFFF;
	    levl_peak_value=0;
	    MakeLevl();
	    lseek(wave_file.handle(),0,SEEK_END);
	    CheckExitCode("RDWaveFile::closeWave()",
			  write(wave_file.handle(),"levl",4));
	    lsize=LEVL_CHUNK_SIZE+energy_data.size()*2-8;
	    size_buf[0]=lsize&0xff;
	    size_buf[1]=(lsize>>8)&0xff;
	    size_buf[2]=(lsize>>16)&0xff;
	    size_buf[3]=(lsize>>24)&0xff;
	    CheckExitCode("RDWaveFile::closeWave()",
			  write(wave_file.handle(),size_buf,4));
	    CheckExitCode("RDWaveFile::closeWave()",
			  write(wave_file.handle(),levl_chunk_data,
				LEVL_CHUNK_SIZE-8));
	    // Fixup the endianness
	    unsigned char * sbuf = new unsigned char [2 * energy_data.size()];
	    for (unsigned int i=0; i < energy_data.size(); i++){
	      WriteSword (sbuf,2*i,(unsigned short) energy_data[i]);
	    }
	    CheckExitCode("RDWaveFile::closeWave()",
			  write(wave_file.handle(),sbuf,2*energy_data.size()));
	    delete [] sbuf;
	    CheckExitCode("RDWaveFile::closeWave()",
			  ftruncate(wave_file.handle(),lseek(wave_file.handle(),
							     0,SEEK_CUR)));
	  }

	  //
	  // Update file size
	  //
	  cptr=lseek(wave_file.handle(),0,SEEK_END)-8;
	  size_buf[0]=cptr&0xff;
	  size_buf[1]=(cptr>>8)&0xff;
	  size_buf[2]=(cptr>>16)&0xff;
	  size_buf[3]=(cptr>>24)&0xff;
	  lseek(wave_file.handle(),4,SEEK_SET);
	  CheckExitCode("RDWaveFile::closeWave()",
			write(wave_file.handle(),size_buf,4));
	  
	  //
	  // Update data chunk size
	  //
	  size_buf[0]=data_length&0xff;
	  size_buf[1]=(data_length>>8)&0xff;
	  size_buf[2]=(data_length>>16)&0xff;
	  size_buf[3]=(data_length>>24)&0xff;
	  lseek(wave_file.handle(),
		FindChunk(wave_file.handle(),"data",&csize)-4,SEEK_SET);
	  CheckExitCode("RDWaveFile::closeWave()",
			write(wave_file.handle(),size_buf,4));
	  
	  //
	  // Update fact chunk
	  //
	  if(FindChunk(wave_file.handle(),"fact",&csize)>0) {
	    if(samples<0) {
	      if(format_tag==WAVE_FORMAT_PCM) {
		samples=data_length/block_align;
	      }
	      if(format_tag==WAVE_FORMAT_MPEG) {
		samples=(unsigned)(1152.0*((double)data_length/
					   (144.0*(double)head_bit_rate/
					    (double)samples_per_sec)));
	      }
	    }
	    size_buf[0]=samples&0xff;
	    size_buf[1]=(samples>>8)&0xff;
	    size_buf[2]=(samples>>16)&0xff;
	    size_buf[3]=(samples>>24)&0xff;
	    WriteChunk(wave_file.handle(),"fact",size_buf,4);
	  }
	  
	  //
	  // Update Cart Chunk
	  //
	  if(cart_chunk) {
	    MakeCart(ptr_offset_msecs);
	    WriteChunk(wave_file.handle(),"cart",cart_chunk_data,CART_CHUNK_SIZE);
	  }
	  
	  //
	  // Update Bext Chunk
	  //
	  if(bext_chunk) {
	    MakeBext();
	    WriteChunk(wave_file.handle(),"bext",bext_coding_data,bext_coding_size);
	  }
	  
	  //
	  // Update Mext Chunk
	  //
	  if(mext_chunk) {
	    MakeMext();
	    WriteChunk(wave_file.handle(),"mext",mext_chunk_data,MEXT_CHUNK_SIZE);
	  }
	  
	  //
	  // Truncate
	  //
	  if(!levl_chunk) {
	    CheckExitCode("RDWaveFile::closeWave()",
			  ftruncate(wave_file.handle(),
				    FindChunk(wave_file.handle(),
					      "data",&csize)+data_length));
	  }
	  else {
	    if((format_tag==WAVE_FORMAT_MPEG)&&(head_layer!=2)) {
	    CheckExitCode("RDWaveFile::closeWave()",
			  ftruncate(wave_file.handle(),
				    FindChunk(wave_file.handle(),
					      "data",&csize)+data_length));
	    }
	  }
	  break;

	case RDWaveFile::Ogg:
#ifdef HAVE_VORBIS
	  WriteOggBuffer(NULL,0);
	  ogg_stream_clear(&ogg_stream);
	  vorbis_block_clear(&vorbis_blk);
	  vorbis_dsp_clear(&vorbis_dsp);
	  vorbis_info_clear(&vorbis_inf);
	  wave_file.close();
#endif  // HAVE_VORBIS
	  break;

	default:
	  break;
    }
  }
  if(wave_type==RDWaveFile::Ogg) {
#ifdef HAVE_VORBIS
    if(!recordable) {
      ov_clear(&vorbis_file);
    }
#endif  // HAVE_VORBIS
  }
  wave_file.close();
  recordable=false;
  time_length=0;
  format_chunk=false;
  format_tag=0;
  channels=0;
  normalize_level=1.0;
  samples_per_sec=0;
  avg_bytes_per_sec=0;
  block_align=0;
  bits_per_sample=0;
  cb_size=0;
  head_layer=0;
  head_bit_rate=0;
  head_mode=0;
  head_mode_ext=0;
  head_emphasis=1;
  head_flags=0;
  mpeg_frame_size=0;
  id3v1_tag=false;
  id3v2_tag[0]=false;
  id3v2_tag[1]=false;
  id3v2_offset[0]=0;
  id3v2_offset[1]=0;
  for(int i=0;i<FMT_CHUNK_SIZE;i++) {
    fmt_chunk_data[i]=0;
  }
  fmt_size=0;
  fact_chunk=false;
  for(int i=0;i<FACT_CHUNK_SIZE;i++) {
    fact_chunk_data[i]=0;
  }
  sample_length=0;
  data_chunk=false;
  data_length=0;
  cart_chunk=false;
  cart_version=0;
  cart_title="";
  cart_title="";
  cart_artist="";
  cart_cut_id="";
  cart_client_id="";
  cart_category="";
  cart_classification="";
  cart_out_cue="";
  cart_start_date=QDate();
  cart_start_time=QTime();
  cart_end_date=QDate();
  cart_end_time=QTime();
  cart_producer_app_id="";
  cart_producer_app_ver="";
  cart_user_def="";
  cart_url="";
  cart_tag_text=""; 
  cart_level_ref=CART_DEFAULT_LEVEL_REF;
  for(int i=0;i<CART_CHUNK_SIZE;i++) {
    cart_chunk_data[i]=0;
  }
  bext_chunk=false;
  bext_description="";
  bext_originator="";
  bext_originator_ref="";
  bext_origination_date=QDate::currentDate();
  bext_origination_time=QTime::currentTime();
  bext_time_reference_low=0;
  bext_time_reference_high=0;
  bext_version=0;
  for(int i=0;i<64;i++) {
    bext_umid[i]=0;
  }
  bext_coding_history="";
  for(int i=0;i<BEXT_CHUNK_SIZE;i++) {
    bext_chunk_data[i]=0;
  }
  free(bext_coding_data);
  bext_coding_data=NULL;
  bext_coding_size=0;
  mext_chunk=false;
  mext_homogenous=true;
  mext_padding_used=false;
  mext_rate_hacked=false;
  mext_free_format=false;
  mext_frame_size=0;
  mext_anc_length=0;
  mext_left_energy=false;
  mext_ancillary_private=false;
  mext_right_energy=false;
  for(int i=0;i<MEXT_CHUNK_SIZE;i++) {
    mext_chunk_data[i]=0;
  }
  levl_chunk=false;
  levl_format=DEFAULT_LEVL_FORMAT; 
  levl_points=DEFAULT_LEVL_POINTS;
  levl_block_size=DEFAULT_LEVL_BLOCK_SIZE;
  energy_loaded=false;
  energy_data.clear();
  free(cook_buffer);
  cook_buffer=NULL;
  cook_buffer_size=0;
  encode_quality=5.0f;
  serial_number=-1;
  atx_offset=0;
  av10_chunk=false;
  rdxl_chunk=false;
}


void RDWaveFile::resetWave()
{
  if(wave_type!=RDWaveFile::Ogg) {
    lseek(wave_file.handle(),data_start,SEEK_SET);
    CheckExitCode("RDWaveFile::resetWave()",
		  ftruncate(wave_file.handle(),data_start));
  }
}


QString RDWaveFile::getName() const
{
  return wave_file_name;
}


bool RDWaveFile::getFormatChunk() const
{
  return format_chunk;
}



bool RDWaveFile::getFactChunk() const
{
  return fact_chunk;
}



unsigned RDWaveFile::getSampleLength() const
{
  return sample_length;
}



unsigned RDWaveFile::getTimeLength() const
{
  return time_length;
}


unsigned RDWaveFile::getExtTimeLength() const
{
  return ext_time_length;
}


bool RDWaveFile::getDataChunk() const
{
  return data_chunk;
}



unsigned RDWaveFile::getDataLength() const
{
  return data_length;
}


int RDWaveFile::readWave(void *buf,int count)
{
  unsigned int pos;
  int c = 0;
#ifdef HAVE_VORBIS
  int stream;
  int n;
  int16_t *sample;
#endif  // HAVE_VORBIS

  switch(wave_type) {
      case RDWaveFile::Ogg:
#ifdef HAVE_VORBIS
        n = 0;
        while(n!=count){
          int ret = ov_read(&vorbis_file,(char *)buf+n,count-n,0,2,1,&stream);
	  if (!ret) break;
	    n+=ret;
	}
        if(normalize_level != 1.0f){
          for (int i=0;i<n/2;i++) {
            sample=(int16_t *)buf+i;
            *sample=(int16_t)(normalize_level*(double)*sample);
	  }
        }
	return n;
#endif  // HAVE_VORBIS
	return 0;

      case RDWaveFile::Wave:
	pos = lseek(wave_file.handle(),0,SEEK_CUR);
	//
	// FIXME: how fix comparing singed (data_start, count) vs. 
	// unsigned (pos, data_length) ... WAVE standard is 32 bit, 
	// but not sure if signed or not... 
	// grauf@rfa.org Tue, 04 Apr 2006 21:02:51 -0400
	//
        if (((pos+count)>(data_start+data_length))&&(data_length>0)) {
          count=count - ( (pos+count) - (data_start+data_length) );
        }
	c = read(wave_file.handle(),buf,count);
	break;
      default:
	c = read(wave_file.handle(),buf,count);
  }
  if ( c <0 ) return 0; // read error
  // Fixup the buffer for big endian hosts (Wav is defined as LE).
  if (htonl (1l) == 1){ // Big endian host
    for(int i = 0; i < c/2; i++){
      ((short*)buf)[i] = ReadSword((unsigned char *)buf,2*i);
    }
  }

  //  printf("RDWaveFile: 0x%02X\n",((char *)buf)[0]&0xff);

  return c;
}


int RDWaveFile::writeWave(void *buf,int count)
{
  if(!recordable) {
    return -1;
  }
  switch(format_tag) {
  case WAVE_FORMAT_PCM:
    switch(bits_per_sample) {
    case 16:
      if(levl_chunk) {
	for(int i=0;i<count;i++) {
	  switch(levl_istate) {
	  case 0:   // Left Channel, LSB
	    levl_accum=((char *)buf)[i]&0xff;
	    levl_istate=1;
	    break;

	  case 1:   // Left Channel, MSB
	    levl_accum|=((((char *)buf)[i]&0xff)<<8);
	    switch(channels) {
	    case 1:
	      if(levl_accum>energy_data[energy_data.size()-1]) {
		energy_data[energy_data.size()-1]=levl_accum;
	      }
	      if(++levl_block_ptr==1152) {
		energy_data.push_back(0);
		levl_block_ptr=0;
	      }
	      levl_istate=0;
	      break;

	    case 2:
	      if(levl_accum>energy_data[energy_data.size()-2]) {
		energy_data[energy_data.size()-2]=levl_accum;
	      }
	      levl_istate=2;
	      break;
	    }
	    break;

	  case 2:   // Right Channel, LSB
	    levl_accum=((char *)buf)[i]&0xff;
	    levl_istate=3;
	    break;

	  case 3:   // Right Channel, MSB
	    levl_accum|=((((char *)buf)[i]&0xff)<<8);
	    if(levl_accum>energy_data[energy_data.size()-1]) {
	      energy_data[energy_data.size()-1]=levl_accum;
	    }
	    if(++levl_block_ptr==1152) {
	      energy_data.push_back(0);
	      energy_data.push_back(0);
	      levl_block_ptr=0;
	    }
	    levl_istate=0;
	    break;
	  }
	}
      }
      lseek(wave_file.handle(),0,SEEK_END);
      data_length+=count;
      // Fixup the buffer for big endian hosts (Wav is defined as LE).
      if (htonl (1l) == 1l){ // Big endian host
	for(int i = 0; i < count/2; i++) {
	  unsigned short s = ((unsigned short*)buf)[i];
	  WriteSword((unsigned char *)buf,2*i,s);
	}
      }
      return write(wave_file.handle(),buf,count);

    case 24:
      if(levl_chunk) {
	for(int i=0;i<count;i++) {
	  switch(levl_istate) {
	  case 0:   // Left Channel LSB
	    levl_istate=1;
	    break;

	  case 1:   // Left Channel, Middle Byte
	    levl_accum=((char *)buf)[i]&0xff;
	    levl_istate=2;
	    break;

	  case 2:   // Left Channel, MSB
	    levl_accum|=((((char *)buf)[i]&0xff)<<8);
	    switch(channels) {
	    case 1:
	      if(levl_accum>energy_data[energy_data.size()-1]) {
		energy_data[energy_data.size()-1]=levl_accum;
	      }
	      if(++levl_block_ptr==1152) {
		energy_data.push_back(0);
		levl_block_ptr=0;
	      }
	      levl_istate=0;
	      break;

	    case 2:
	      if(levl_accum>energy_data[energy_data.size()-2]) {
		energy_data[energy_data.size()-2]=levl_accum;
	      }
	      levl_istate=3;
	      break;
	    }
	    break;

	  case 3:  // Right Channel, LSB
	    levl_istate=4;
	    break;

	  case 4:   // Right Channel, Middle
	    levl_accum=((char *)buf)[i]&0xff;
	    levl_istate=5;
	    break;

	  case 5:   // Right Channel, MSB
	    levl_accum|=((((char *)buf)[i]&0xff)<<8);
	    if(levl_accum>energy_data[energy_data.size()-1]) {
	      energy_data[energy_data.size()-1]=levl_accum;
	    }
	    if(++levl_block_ptr==1152) {
	      energy_data.push_back(0);
	      energy_data.push_back(0);
	      levl_block_ptr=0;
	    }
	    levl_istate=0;
	    break;
	  }
	}
      }
      lseek(wave_file.handle(),0,SEEK_END);
      data_length+=count;
      return write(wave_file.handle(),buf,count);
    }

  case WAVE_FORMAT_MPEG:
    if(levl_chunk&&(head_layer==2)) {
      for(int i=0;i<count;i++) {
	if(levl_block_ptr==(block_align-5)) {  // Right Channel, MSB
	  if(channels==2) {
	    levl_accum=((((char *)buf)[i]&0xff)<<8);
	  }
	  levl_block_ptr++;
	}
	else {
	  if(levl_block_ptr==(block_align-4)) {  // Right Channel, LSB
	    if(channels==2) {
	      levl_accum|=((char *)buf)[i]&0xff;
	      energy_data[energy_data.size()-1]=levl_accum;
	    }
	    levl_block_ptr++;
	  }
	  else {
	    if(levl_block_ptr==(block_align-2)) { // Left Channel, MSB
	      levl_accum=((((char *)buf)[i]&0xff)<<8);
	      levl_block_ptr++;
	    }
	    else {
	      if(levl_block_ptr==(block_align-1)) {  // Left Channel, LSB
		levl_accum|=((char *)buf)[i]&0xff;
		energy_data[energy_data.size()-channels]=levl_accum;
		for(unsigned j=0;j<channels;j++) {
		  energy_data.push_back(0);
		}
		levl_block_ptr=0;
	      }
	      else {
		levl_block_ptr++;
	      }
	    }
	  }
	}
      }
    }
    lseek(wave_file.handle(),0,SEEK_END);
    data_length+=count;
    return write(wave_file.handle(),buf,count);

  case WAVE_FORMAT_VORBIS:
    WriteOggBuffer((char *)buf,count);
    break;
  }
  return 0;
}


bool RDWaveFile::hasEnergy()
{
  GetEnergy();
  return has_energy;
}


unsigned RDWaveFile::energySize()
{
  GetEnergy();
  if(!has_energy) {
    return 0;
  }
  return energy_data.size();
}


unsigned short RDWaveFile::energy(unsigned frame)
{
  if(!has_energy) {
    return 0;
  }
  GetEnergy();
  return energy_data[frame];
}


int RDWaveFile::readEnergy(unsigned short buf[],int count)
{
  if(!has_energy) {
    return 0;
  }
  GetEnergy();
  for(int i=0;i<count;i++) {
    if((i+energy_ptr)<energy_data.size()) {
      buf[i]=energy_data[i+energy_ptr];
    }
    else {
      energy_ptr+=i;
      return i;
    }
  }
  return 0;
}


int RDWaveFile::startTrim(int level)
{
  double ratio=pow(10,-(double)level/2000.0)*32768.0;
  GetEnergy();
  for(unsigned i=0;i<energy_data.size();i++) {
    if((double)energy_data[i]>=ratio) {
      return i*1152/getChannels();
    }
  }
  return -1;
}


int RDWaveFile::endTrim(int level)
{
  double ratio=pow(10,-(double)level/2000.0)*32768.0;
  GetEnergy();
  for(int i=energy_data.size()-1;i>=0;i--) {
    if((double)energy_data[i]>=ratio) {
      return i*1152/getChannels();
    }
  }
  return -1;
}


void RDWaveFile::getSettings(RDSettings *settings)
{
  switch(type()) {
  case RDWaveFile::Pcm8:
  case RDWaveFile::Aiff:
  case RDWaveFile::M4A:
    // FIXME
    break;

  case RDWaveFile::Wave:
    switch(getFormatTag()) {
    case WAVE_FORMAT_PCM:
      settings->setFormat(RDSettings::Pcm16);
      break;

    case WAVE_FORMAT_MPEG:
      settings->setFormat((RDSettings::Format)getHeadLayer());
      break;
    }
    break;

  case RDWaveFile::Mpeg:
  case RDWaveFile::Atx:
  case RDWaveFile::Tmc:
  case RDWaveFile::Ambos:
    settings->setFormat((RDSettings::Format)getHeadLayer());
    break;

  case RDWaveFile::Ogg:
    settings->setFormat(RDSettings::OggVorbis);
    break;
  case RDWaveFile::Flac:
#ifdef HAVE_FLAC
    settings->setFormat(RDSettings::Flac);
#endif  // HAVE_FLAC
    break;
  }
  settings->setChannels(getChannels());
  settings->setSampleRate(getSamplesPerSec());
  settings->setLayer(getHeadLayer());
  settings->setBitRate(getHeadBitRate());
}


void RDWaveFile::setSettings(const RDSettings *settings)
{
}


int RDWaveFile::seekWave(int offset,int whence)
{
  int pos;
  unsigned abspos;

  switch(wave_type) {
      case RDWaveFile::Ogg:
#ifdef HAVE_VORBIS
	switch(whence) {
	    case SEEK_SET:
	      if(ov_pcm_seek(&vorbis_file,offset/(2*channels))==0) {
		return offset;
	      }
	      return -1;
	      break;

	    case SEEK_CUR:
	      pos=ov_pcm_tell(&vorbis_file)*2*channels;
	      if(offset==0) {
		return pos;
	      }
	      return seekWave(pos+offset,SEEK_SET);
	      break;

	    case SEEK_END:
	      break;
	}
#endif  // HAVE_VORBIS
	return -1;
	break;

      case RDWaveFile::Wave:
        switch(whence) {
            case SEEK_SET:
              if(offset<0) {
                offset=0;
              }
              if((unsigned)offset>data_length) {
                offset=data_length;
              }
              return lseek(wave_file.handle(),
                           offset+data_start,SEEK_SET)-data_start;
              break;

            case SEEK_CUR:
              pos = lseek(wave_file.handle(),0,SEEK_CUR);
	      abspos=pos+offset;
	      if((pos+offset)<0) {
		abspos=0;
	      }
              if (abspos<(unsigned)data_start) {
                offset=offset + (data_start - abspos);
              }
              if (abspos>(data_start+data_length)) { 
                offset=offset - (abspos - (data_start+data_length));
              }
              return lseek(wave_file.handle(),offset,SEEK_CUR)-data_start;
              break;
            case SEEK_END:
              pos = lseek(wave_file.handle(),0,SEEK_END);
	      abspos=pos+offset;
	      if((pos+offset)<0) {
		abspos=0;
	      }
              if (abspos<(unsigned)data_start) {
                offset=offset + (data_start - (pos+offset));
              }
              if (abspos>(data_start+data_length)) {
                offset=offset - (abspos - (data_start+data_length));
              }
              return lseek(wave_file.handle(),offset,SEEK_END)-data_start;
              break;
        }
        break;

      default:
	switch(whence) {
	    case SEEK_SET:
	      return lseek(wave_file.handle(),
			   offset+data_start,SEEK_SET)-data_start;
	      break;
	    case SEEK_CUR:
	      return lseek(wave_file.handle(),offset,SEEK_CUR)-data_start;
	      break;
	    case SEEK_END:
	      return lseek(wave_file.handle(),offset,SEEK_END)-data_start;
	      break;
	}
  }
  return 0;
} 


unsigned short RDWaveFile::getFormatTag() const
{
  return format_tag;
}


void RDWaveFile::setFormatTag(unsigned short format)
{
  if(!recordable) {
    format_tag=format;
  }
}


unsigned short RDWaveFile::getChannels() const
{
  return channels;
}


void RDWaveFile::setChannels(unsigned short chan)
{
  if(!recordable) {
    channels=chan;
  }
}


unsigned RDWaveFile::getSamplesPerSec() const
{
  return samples_per_sec;
}


void RDWaveFile::setSamplesPerSec(unsigned rate)
{
  if(!recordable) {
    samples_per_sec=rate;
  }
}


unsigned RDWaveFile::getAvgBytesPerSec() const
{
  return avg_bytes_per_sec;
}



unsigned short RDWaveFile::getBlockAlign() const
{
  return block_align;
}


unsigned short RDWaveFile::getBitsPerSample() const
{
  return bits_per_sample;
}


void RDWaveFile::setBitsPerSample(unsigned short bits)
{
  if(!recordable) {
    bits_per_sample=bits;
  }
}


unsigned short RDWaveFile::getHeadLayer() const
{
  return head_layer;
}


void RDWaveFile::setHeadLayer(unsigned short layer)
{
  if(!recordable) {
    head_layer=layer;
  }
}


unsigned RDWaveFile::getHeadBitRate() const
{
  return head_bit_rate;
}


void RDWaveFile::setHeadBitRate(unsigned rate)
{
  if(!recordable) {
    head_bit_rate=rate;
  }
}


unsigned short RDWaveFile::getHeadMode() const
{
  return head_mode;
}


void RDWaveFile::setHeadMode(unsigned short mode)
{
  if(!recordable) {
    head_mode=mode;
  }
}


void RDWaveFile::setHeadFlags(unsigned short flags)
{
  if(!recordable) {
    head_flags=flags;
  }
}


unsigned RDWaveFile::getHeadModeExt() const
{
  return head_mode_ext;
}



unsigned RDWaveFile::getHeadEmphasis() const
{
  return head_emphasis;
}



unsigned short RDWaveFile::getHeadFlags() const
{
  return head_flags;
}


bool RDWaveFile::getCartChunk() const
{
  return cart_chunk;
}


void RDWaveFile::setCartChunk(bool state)
{
  if(!recordable) {
    cart_chunk=state;
  }
}


unsigned RDWaveFile::getCartVersion() const
{
  return cart_version;
}


QString RDWaveFile::getCartTitle() const
{
  return cart_title;
}


void RDWaveFile::setCartTitle(QString string)
{
  cart_title=string;
}


QString RDWaveFile::getCartArtist() const
{
  return cart_artist;
}


void RDWaveFile::setCartArtist(QString string)
{
  cart_artist=string;
}


QString RDWaveFile::getCartCutID() const
{
  return cart_cut_id;
}


void RDWaveFile::setCartCutID(QString string)
{
  cart_cut_id=string;
}


QString RDWaveFile::getCartClientID() const
{
  return cart_client_id;
}


void RDWaveFile::setCartClientID(QString string)
{
  cart_client_id=string;
}


QString RDWaveFile::getCartCategory() const
{
  return cart_category;
}


void RDWaveFile::setCartCategory(QString string)
{
  cart_category=string;
}


QString RDWaveFile::getCartClassification() const
{
  return cart_classification;
}


void RDWaveFile::setCartClassification(QString string)
{
  cart_classification=string;
}


QString RDWaveFile::getCartOutCue() const
{
  return cart_out_cue;
}


void RDWaveFile::setCartOutCue(QString string)
{
  cart_out_cue=string;
}


QDate RDWaveFile::getCartStartDate() const
{
  return cart_start_date;
}


void RDWaveFile::setCartStartDate(QDate date)
{
  cart_start_date=date;
}


QTime RDWaveFile::getCartStartTime() const
{
  return cart_start_time;
}


void RDWaveFile::setCartStartTime(QTime time)
{
  cart_start_time=time;
}


QDate RDWaveFile::getCartEndDate() const
{
  return cart_end_date;
}


void RDWaveFile::setCartEndDate(QDate date)
{
  cart_end_date=date;
}


QTime RDWaveFile::getCartEndTime() const
{
  return cart_end_time;
}


void RDWaveFile::setCartEndTime(QTime time)
{
  cart_end_time=time;
}


QString RDWaveFile::getCartProducerAppID() const
{
  return cart_producer_app_id;
}


QString RDWaveFile::getCartProducerAppVer() const
{
  return cart_producer_app_ver;
}


QString RDWaveFile::getCartUserDef() const
{
  return cart_user_def;
}


void RDWaveFile::setCartUserDef(QString string)
{
  cart_user_def=string;
}


unsigned RDWaveFile::getCartLevelRef() const
{
  return cart_level_ref;
}


void RDWaveFile::setCartLevelRef(unsigned level)
{
  cart_level_ref=level;
}


QString RDWaveFile::getCartTimerLabel(int index) const
{
  if(index<MAX_TIMERS) {
    return cart_timer_label[index];
  }
  return QString("");
}


unsigned RDWaveFile::getCartTimerSample(int index) const
{
  if(index<MAX_TIMERS) {
    return cart_timer_sample[index];
  }
  return 0;
}


QString RDWaveFile::getCartURL() const
{
  return cart_url;
}


void RDWaveFile::setCartURL(QString string)
{
  cart_url=string;
}


QString RDWaveFile::getCartTagText() const
{
  return cart_tag_text;
}


bool RDWaveFile::getBextChunk() const
{
  return bext_chunk;
}


void RDWaveFile::setBextChunk(bool state) 
{
  if(!recordable) {
    bext_chunk=state;
  }
}


QString RDWaveFile::getBextDescription() const
{
  return bext_description;
}


void RDWaveFile::setBextDescription(QString string)
{
  bext_description=string;
}


QString RDWaveFile::getBextOriginator() const
{
  return bext_originator;
}


void RDWaveFile::setBextOriginator(QString string)
{
  bext_originator=string;
}


QString RDWaveFile::getBextOriginatorRef() const
{
  return bext_originator_ref;
}


void RDWaveFile::setBextOriginatorRef(QString string) 
{
  bext_originator_ref=string;
}


QDate RDWaveFile::getBextOriginationDate() const
{
  return bext_origination_date;
}


void RDWaveFile::setBextOriginationDate(QDate date)
{
  bext_origination_date=date;
}


QTime RDWaveFile::getBextOriginationTime() const
{
  return bext_origination_time;
}


void RDWaveFile::setBextOriginationTime(QTime time)
{
  bext_origination_time=time;
}


unsigned RDWaveFile::getBextTimeReferenceLow() const
{
  return bext_time_reference_low;
}


void RDWaveFile::setBextTimeReferenceLow(unsigned sample)
{
  bext_time_reference_low=sample;
}


unsigned RDWaveFile::getBextTimeReferenceHigh() const
{
  return bext_time_reference_low;
}


void RDWaveFile::setBextTimeReferenceHigh(unsigned sample)
{
  bext_time_reference_high=sample;
}


unsigned short RDWaveFile::getBextVersion() const
{
  return bext_version;
}


void RDWaveFile::getBextUMD(unsigned char *buf) const
{
  for(int i=0;i<64;i++) {
    buf[i]=bext_umid[i];
  }
}


void RDWaveFile::setBextUMD(unsigned char *buf)
{
  for(int i=0;i<64;i++) {
    bext_umid[i]=buf[i];
  }
}


QString RDWaveFile::getBextCodingHistory() const
{
  return bext_coding_history;
}


void RDWaveFile::setBextCodingHistory(QString string)
{
  if(!recordable) {
    bext_coding_history=string;
  }
}


bool RDWaveFile::getMextChunk() const
{
  return mext_chunk;
}


void RDWaveFile::setMextChunk(bool state)
{
  if(!recordable) {
    mext_chunk=state;
  }
}


bool RDWaveFile::getMextHomogenous() const
{
  return mext_homogenous;
}


void RDWaveFile::setMextHomogenous(bool state)
{
  mext_homogenous=state;
}


bool RDWaveFile::getMextPaddingUsed() const
{
  return mext_padding_used;
}


void RDWaveFile::setMextPaddingUsed(bool state)
{
  mext_padding_used=state;
}


bool RDWaveFile::getMextHackedBitRate() const
{
  return mext_rate_hacked;
}


void RDWaveFile::setMextHackedBitRate(bool state)
{
  mext_rate_hacked=state;
}


bool RDWaveFile::getMextFreeFormat() const
{
  return mext_free_format;
}


void RDWaveFile::setMextFreeFormat(bool state)
{
  mext_free_format=state;
}


int RDWaveFile::getMextFrameSize() const
{
  return mext_frame_size;
}


void RDWaveFile::setMextFrameSize(int size)
{
  mext_frame_size=size;
}


int RDWaveFile::getMextAncillaryLength() const
{
  return mext_anc_length;
}


void RDWaveFile::setMextAncillaryLength(int length)
{
  mext_anc_length=length;
}


bool RDWaveFile::getMextLeftEnergyPresent() const
{
  return mext_left_energy;
}


void RDWaveFile::setMextLeftEnergyPresent(bool state)
{
  mext_left_energy=state;
}


bool RDWaveFile::getMextPrivateDataPresent() const
{
  return mext_ancillary_private;
}


void RDWaveFile::setMextPrivateDataPresent(bool state)
{
  mext_ancillary_private=state;
}


bool RDWaveFile::getMextRightEnergyPresent() const
{
  return mext_right_energy;
}


void RDWaveFile::setMextRightEnergyPresent(bool state)
{
  mext_right_energy=state;
}


bool RDWaveFile::getLevlChunk() const
{
  return levl_chunk;
}


void RDWaveFile::setLevlChunk(bool state)
{
  levl_chunk=state;
}


int RDWaveFile::getLevlVersion() const
{
  return levl_version;
}


void RDWaveFile::setLevlVersion(unsigned ver)
{
  levl_version=ver;
}


int RDWaveFile::getLevlBlockSize() const
{
  return levl_block_size;
}


void RDWaveFile::setLevlBlockSize(unsigned size)
{
  levl_block_size=size;
}


int RDWaveFile::getLevlChannels() const
{
  return levl_channels;
}


unsigned short RDWaveFile::getLevlPeak() const
{
  return levl_peak_value;
}


QDateTime RDWaveFile::getLevlTimestamp() const
{
  return levl_timestamp;
}


void RDWaveFile::setEncodeQuality(float qual)
{
  encode_quality=qual;
}


int RDWaveFile::getSerialNumber() const
{
  return serial_number;
}


void RDWaveFile::setSerialNumber(int serial)
{
  serial_number=serial;
}


bool RDWaveFile::getScotChunk() const
{
  return scot_chunk;
}


bool RDWaveFile::getAIR1Chunk() const
{
  return AIR1_chunk;
}


bool RDWaveFile::getRdxlChunk() const
{
  return rdxl_chunk;
}


QString RDWaveFile::getRdxlContents() const
{
  return rdxl_contents;
}


void RDWaveFile::setRdxlContents(const QString &xml)
{
  rdxl_contents=xml;

  //
  // Make sure that the RDXL chunk is of even length.
  // (To avoid goosing a bug in CoolEdit Pro 2003).
  //
  if((rdxl_contents.length()%2)!=0) {
    rdxl_contents+=" ";
  }
}


QString RDWaveFile::formatText(RDWaveFile::Format fmt)
{
  QString ret=QObject::tr("unknown");

  switch(fmt) {
  case RDWaveFile::Pcm8:
    ret=QObject::tr("PCM8");
    break;

  case RDWaveFile::Pcm16:
    ret=QObject::tr("PCM16");
    break;

  case RDWaveFile::Float32:
    ret=QObject::tr("Float32");
    break;

  case RDWaveFile::MpegL1:
    ret=QObject::tr("MPEG Layer I");
    break;

  case RDWaveFile::MpegL2:
    ret=QObject::tr("MPEG Layer II (MP2)");
    break;

  case RDWaveFile::MpegL3:
    ret=QObject::tr("MPEG Layer III (MP3)");
    break;

  case RDWaveFile::DolbyAc2:
    ret=QObject::tr("Dolby AC2");
    break;

  case RDWaveFile::DolbyAc3:
    ret=QObject::tr("Dolby AC3");
    break;

  case RDWaveFile::Vorbis:
    ret=QObject::tr("OggVorbis");
    break;

  case RDWaveFile::Pcm24:
    ret=QObject::tr("PCM24");
    break;

  }

  return ret;
}


QString RDWaveFile::typeText(RDWaveFile::Type type)
{
  QString ret=QObject::tr("unknown");

  switch(type) {
  case RDWaveFile::Unknown:
    break;

  case RDWaveFile::Wave:
    ret=QObject::tr("RIFF/WAVE");
    break;

  case RDWaveFile::Mpeg:
    ret=QObject::tr("Raw MPEG Bitstream");
    break;

  case RDWaveFile::Ogg:
    ret=QObject::tr("Ogg Bitstream");
    break;

  case RDWaveFile::Atx:
    ret=QObject::tr("ATX");
    break;

  case RDWaveFile::Tmc:
    ret=QObject::tr("TMC");
    break;

  case RDWaveFile::Flac:
    ret=QObject::tr("Raw FLAC Bitstream");
    break;

  case RDWaveFile::Ambos:
    ret=QObject::tr("AM-BOS Hybrid");
    break;

  case RDWaveFile::Aiff:
    ret=QObject::tr("AIFF");
    break;

  case RDWaveFile::M4A:
    ret=QObject::tr("MP4");
    break;
  }

  return ret;
}


RDWaveFile::Type RDWaveFile::GetType(int fd)
{
  if(IsWav(fd)) {
    return RDWaveFile::Wave;
  }
  if(IsAiff(fd)) {
    return RDWaveFile::Aiff;
  }
  if(IsFlac(fd)) {
    return RDWaveFile::Flac;
  }
  if(IsAtx(fd)) {
    return RDWaveFile::Atx;
  }
  if(IsTmc(fd)) {
    return RDWaveFile::Tmc;
  }
  if(IsOgg(fd)) {
    return RDWaveFile::Ogg;
  }
  if(IsM4A(fd)) {
    return RDWaveFile::M4A;
  }
  if(IsMpeg(fd)) {
    return RDWaveFile::Mpeg;
  }
  return RDWaveFile::Unknown;
}


bool RDWaveFile::IsWav(int fd)
{
  int i;
  char buffer[5];
  
  /* 
   * Is this a riff file? 
   */
  lseek(fd,0,SEEK_SET);
  i=read(fd,buffer,4);
  if(i==4) {
    buffer[4]=0;
    if(strcmp("RIFF",buffer)!=0) {
      return false;
    }
  }
  else {
    return false;
  }

  /* 
   * Is this a WAVE file? 
   */
  if(lseek(fd,8,SEEK_SET)!=8) {
    return false;
  }
  i=read(fd,buffer,4);
  if(i==4) {
    buffer[4]=0;
    if(strcmp("WAVE",buffer)!=0) {
      return false;
    }
  }
  else {
    return false;
  }

  return true;
}


bool RDWaveFile::IsMpeg(int fd)
{
  int i;
  unsigned char buffer[11];

  id3v1_tag=false;
  id3v2_tag[0]=false;
  id3v2_tag[1]=false;
  id3v2_offset[0]=0;
  id3v2_offset[1]=0;

  lseek(fd,0,SEEK_SET);
  if((i=read(fd,buffer,10))!=10) {
    return false;
  }
  buffer[3]=0;
  if(!strcasecmp((char *)buffer,"ID3")) {
    id3v2_tag[0]=true;
    id3v2_offset[0]=
      (buffer[9]|(buffer[8]<<7)|(buffer[7]<<14)|(buffer[6]<<21))+10;
  }  
  lseek(fd,id3v2_offset[0],SEEK_SET);
  if((i=read(fd,buffer,2))!=2) {
    return false;
  }
  if((buffer[0]==0xFF)&&((buffer[1]&0xE0)==0xE0)) {
    return true;
  }
  while(read(fd,buffer,1)==1) {
    if(buffer[0]==0xFF) {  // Could be it -- check the next byte
      if(read(fd,buffer,1)==1) {
	if((buffer[0]&0xF0)==0xF0) {  // Got it -- fix things up
	  id3v2_tag[0]=true;
	  id3v2_offset[0]=lseek(fd,0,SEEK_CUR)-2;
	  return true;
	}
      }
    }
  }
  return false;
}


bool RDWaveFile::IsOgg(int fd)
{
#ifdef HAVE_VORBIS
  lseek(fd,0,SEEK_SET);
  if(ov_open(fdopen(fd,"r"),&vorbis_file,NULL,0)==0) {
    return true;
  }
#endif  // HAVE_VORBIS
  return false;
}


bool RDWaveFile::IsAtx(int fd)
{
  char buffer[6];

  lseek(fd,0,SEEK_SET);
  if(read(fd,buffer,5)!=5) {
    return false;
  }
  buffer[5]=0;
  if(strcmp("FILE:",buffer)) {
    return false;
  }

  return true;
}


bool RDWaveFile::IsTmc(int fd)
{
  unsigned char buffer[7];

  lseek(fd,0,SEEK_SET);
  if(read(fd,buffer,6)!=6) {
    return false;
  }
  buffer[6]=0;
  if((buffer[4]!=0xFF)||((buffer[5]&0xF0)!=0xF0)) {
    return false;
  }
  return true;
}


bool RDWaveFile::IsFlac(int fd)
{
#ifdef HAVE_FLAC
  char buffer[5];

  ID3_Tag id3_tag(wave_file_name.toUtf8());
  lseek(fd,id3_tag.GetPrependedBytes(),SEEK_SET);
  if(read(fd,buffer,4)!=4) {
    return false;
  }
  buffer[4]=0;
  if(strcmp(buffer,"fLaC")) {
    return false;
  }
  return true;
#else
  return false;
#endif  // HAVE_FLAC
}


bool RDWaveFile::IsAiff(int fd)
{
  int i;
  char buffer[5];
  
  lseek(fd,0,SEEK_SET);
  i=read(fd,buffer,4);
  if(i==4) {
    buffer[4]=0;
    if(strcmp("FORM",buffer)!=0) {
      return false;
    }
  }
  else {
    return false;
  }

  if(lseek(fd,8,SEEK_SET)!=8) {
    return false;
  }
  i=read(fd,buffer,4);
  if(i==4) {
    buffer[4]=0;
    if(strcmp("AIFF",buffer)!=0) {
      return false;
    }
  }
  else {
    return false;
  }

  return true;
}

bool RDWaveFile::IsM4A(int fd)
{
#ifdef HAVE_MP4_LIBS
  if(!dlmp4.load())
    return false;
  MP4FileHandle f = dlmp4.MP4Read(getName().toUtf8());
  bool ret = f != MP4_INVALID_FILE_HANDLE;
  if(ret)
    dlmp4.MP4Close(f, 0);
  return ret;
#else
  return false;
#endif
}


off_t RDWaveFile::FindChunk(int fd,const char *chunk_name,unsigned *chunk_size,
			    bool big_end)
{
  int offset;
  char name[5]={0,0,0,0,0};
  unsigned char buffer[4];

  lseek(fd,12,SEEK_SET);
  offset=read(fd,name,4);
  if(!isalnum(0xff&name[0])) {
    name[0]=name[1];
    name[1]=name[2];
    name[2]=name[3];
    offset=read(fd,name+3,1);
  }
  offset=read(fd,buffer,4);
  if(big_end) {
    *chunk_size=
      buffer[3]+(256*buffer[2])+(65536*buffer[1])+(16777216*buffer[0]);
  }
  else {
    *chunk_size=
      buffer[0]+(256*buffer[1])+(65536*buffer[2])+(16777216*buffer[3]);
  }
  while(offset==4) {
    if(strcasecmp(chunk_name,name)==0) {
      return lseek(fd,0,SEEK_CUR);
    }
    lseek(fd,*chunk_size,SEEK_CUR);
    offset=read(fd,name,4);
    //
    // Attempt to work around known fencepost errors in WAV files generated by
    // "Sonic Studio soundBlade" by "PME Mastering, Inc." (and perhaps others?)
    //
    if(!isalnum(0xff&name[0])) {
      name[0]=name[1];
      name[1]=name[2];
      name[2]=name[3];
      offset=read(fd,name+3,1);
    }
    offset=read(fd,buffer,4);
    if(big_end) {
      *chunk_size=buffer[3]+(256*buffer[2])+(65536*buffer[1])+
	(16777216*buffer[0]);
    }
    else {
      *chunk_size=buffer[0]+(256*buffer[1])+(65536*buffer[2])+
	(16777216*buffer[3]);
    }
  }
  return -1;
}


bool RDWaveFile::GetChunk(int fd,const char *chunk_name,unsigned *chunk_size,
			  unsigned char *chunk,size_t size,bool big_end)
{
  off_t pos;
  if((pos=FindChunk(fd,chunk_name,chunk_size,big_end))<0) {
    return false;
  }
  lseek(fd,SEEK_SET,pos);
  CheckExitCode("RDWaveFile::GetChunk()",read(fd,chunk,size));
  return true;
}


void RDWaveFile::WriteChunk(int fd,const char *cname,unsigned char *buf,
			    unsigned size,bool big_end)
{
  unsigned char size_buf[4];
  unsigned csize;

  if(FindChunk(fd,cname,&csize)<0) {
    if(big_end) {
      size_buf[3]=size&0xff;
      size_buf[2]=(size>>8)&0xff;
      size_buf[1]=(size>>16)&0xff;
      size_buf[0]=(size>>24)&0xff;
    }
    else {
      size_buf[0]=size&0xff;
      size_buf[1]=(size>>8)&0xff;
      size_buf[2]=(size>>16)&0xff;
      size_buf[3]=(size>>24)&0xff;
    }
    lseek(fd,0,SEEK_END);
    CheckExitCode("RDWaveFile::WriteChunk()",write(fd,cname,4));
    CheckExitCode("RDWaveFile::WriteChunk()",write(fd,size_buf,4));
    CheckExitCode("RDWaveFile::WriteChunk()",write(fd,buf,size));

    return;
  }
  if(csize==size) {
    CheckExitCode("RDWaveFile::WriteChunk()",write(fd,buf,size));
    return;
  }
  //printf("WARNING: Updated chunk size mismatch!  Update not written.\n");
}


void RDWaveFile::WriteChunk(int fd,const char *cname,const QString &contents)
{
  unsigned char size_buf[4];
  size_buf[0]=contents.toUtf8().length()&0xff;
  size_buf[1]=(contents.toUtf8().length()>>8)&0xff;
  size_buf[2]=(contents.toUtf8().length()>>16)&0xff;
  size_buf[3]=(contents.toUtf8().length()>>24)&0xff;

  lseek(fd,0,SEEK_END);
  CheckExitCode("RDWaveFile::WriteChunk()",write(fd,cname,4));
  CheckExitCode("RDWaveFile::WriteChunk()",write(fd,size_buf,4));
  CheckExitCode("RDWaveFile::WriteChunk()",
		write(fd,contents.toUtf8(),contents.toUtf8().length()));
}


bool RDWaveFile::GetFmt(int fd)
{
  unsigned chunk_size;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"fmt ",&chunk_size,fmt_chunk_data,FMT_CHUNK_SIZE)) {
    format_chunk=false;
    return false;
  }
  format_chunk=true;

  /*
   * Generic data -- present in ALL valid WAV files
   */
  format_tag=fmt_chunk_data[0]+256*fmt_chunk_data[1];
  channels=fmt_chunk_data[2]+256*fmt_chunk_data[3];
  samples_per_sec=fmt_chunk_data[4]+256*fmt_chunk_data[5]+65536*
    fmt_chunk_data[6]+16777216*fmt_chunk_data[7];
  avg_bytes_per_sec=fmt_chunk_data[8]+256*fmt_chunk_data[9]+65536*
    fmt_chunk_data[10]+16777216*fmt_chunk_data[11];
  block_align=fmt_chunk_data[12]+256*fmt_chunk_data[13];
  
  /*
   * PCM Linear specific fields
   */
  if(format_tag==WAVE_FORMAT_PCM) {
    bits_per_sample=fmt_chunk_data[14]+256*fmt_chunk_data[15];
  }

  /*
   * MPEG-1 specific fields
   */
  if(format_tag==WAVE_FORMAT_MPEG) {
    head_layer=fmt_chunk_data[18]+256*fmt_chunk_data[19];
    head_bit_rate=fmt_chunk_data[20]+256*fmt_chunk_data[21]+65536*
      fmt_chunk_data[22]+16777216*fmt_chunk_data[23];
    head_mode=fmt_chunk_data[24]+256*fmt_chunk_data[25];
    head_mode_ext=fmt_chunk_data[26]+256*fmt_chunk_data[27];
    head_emphasis=fmt_chunk_data[28]+256*fmt_chunk_data[29];
    head_flags=fmt_chunk_data[30]+256*fmt_chunk_data[31];
  }

  if(format_tag==WAVE_FORMAT_MPEGLAYER3) {
    if(!GetChunk(wave_file.handle(),"data",&data_length,NULL,0)) {
      return false;
    }
    data_start=lseek(wave_file.handle(),0,SEEK_CUR);
    GetMpegHeader(fd,data_start);
    format_tag=WAVE_FORMAT_MPEG;
  }

  return true;
}



bool RDWaveFile::GetFact(int fd)
{
  unsigned chunk_size;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"fact",&chunk_size,fact_chunk_data,FACT_CHUNK_SIZE)) {
    return false;
  }
  fact_chunk=true;

  sample_length=fact_chunk_data[0]+256*fact_chunk_data[1]+
    65536*fact_chunk_data[2]+16777216*fact_chunk_data[3];
  return true;
}



bool RDWaveFile::GetCart(int fd)
{
  unsigned chunk_size;
  char *tag_buffer=NULL;
  int i,j;
  QMultiMap<QString,int> timer_map;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"cart",&chunk_size,cart_chunk_data,CART_CHUNK_SIZE)) {
    return false;
  }
  cart_chunk=true;

  cart_version=cart_chunk_data[0]+256*cart_chunk_data[1]+
    65536*cart_chunk_data[2]+16777216*cart_chunk_data[3];
  cart_title=cutString((char *)cart_chunk_data,4,64);
  cart_artist=cutString((char *)cart_chunk_data,68,64);
  cart_cut_id=cutString((char *)cart_chunk_data,132,64);
  cart_client_id=cutString((char *)cart_chunk_data,196,64);
  cart_category=cutString((char *)cart_chunk_data,260,64);
  cart_classification=cutString((char *)cart_chunk_data,324,64);
  cart_out_cue=cutString((char *)cart_chunk_data,388,64);
  cart_start_date=cutDate((char *)cart_chunk_data,452);
  cart_start_time=cutTime((char *)cart_chunk_data,462);
  cart_end_date=cutDate((char *)cart_chunk_data,470);
  cart_end_time=cutTime((char *)cart_chunk_data,480);
  cart_producer_app_id=cutString((char *)cart_chunk_data,488,64);
  cart_producer_app_ver=cutString((char *)cart_chunk_data,552,64);
  cart_user_def=cutString((char *)cart_chunk_data,616,64);
  cart_level_ref=cart_chunk_data[680]+256*cart_chunk_data[681]+
    65536*cart_chunk_data[682]+16777216*cart_chunk_data[683];

  j=0;
  for(i=0;i<MAX_TIMERS;i++) {
    if(cart_chunk_data[684+i*8]!=0) {
      cart_timer_label[j]=cutString((char *)cart_chunk_data,684+i*8,4);
      cart_timer_sample[j]=cart_chunk_data[688+i*8]+
	256*cart_chunk_data[689+i*8]+
	65536*cart_chunk_data[690+i*8]+
	16777216*cart_chunk_data[691+i*8];
      j++;
    }
  }
  for(i=0;i<MAX_TIMERS;i++) {
    if(!cart_timer_label[i].isEmpty()) {
      timer_map.insert(cart_timer_label[i],cart_timer_sample[i]);
    }
  }
  if(wave_type==RDWaveFile::Wave) {
    cart_url=cutString((char *)cart_chunk_data,1024,1024);

    //
    // Get the Tag Text
    //
    if(chunk_size>2048) {
      tag_buffer=(char *)malloc(chunk_size-2048+1);
      CheckExitCode("RDWaveFile::GetCart()",
		    read(wave_file.handle(),tag_buffer,chunk_size-2048));
      tag_buffer[chunk_size-2048]=0;
      cart_tag_text=tag_buffer;
      free(tag_buffer);
      tag_buffer=NULL;
    }
  }
  else {
    cart_url="";
    cart_tag_text="";
  }


  if(wave_data!=NULL) {
    wave_data->setMetadataFound(true);

    //
    // Process Text Metadata
    //
    wave_data->setTitle(QString(cart_title).remove(QChar(0)));
    wave_data->setArtist(QString(cart_artist).remove(QChar(0)));
    wave_data->setCutId(cart_cut_id);
    wave_data->setClient(QString(cart_client_id).remove(QChar(0)));
    wave_data->setCategory(QString(cart_category).remove(QChar(0)));
    wave_data->setClassification(QString(cart_classification).remove(QChar(0)));
    wave_data->setOutCue(QString(cart_out_cue).remove(QChar(0)));
    wave_data->setStartDate(cart_start_date);
    wave_data->setStartTime(cart_start_time);
    wave_data->setEndDate(cart_end_date);
    wave_data->setEndTime(cart_end_time);
    wave_data->setUserDefined(QString(cart_user_def).remove(QChar(0)));
    wave_data->setUrl(QString(cart_url).remove(QChar(0)));
    wave_data->setTagText(QString(cart_tag_text).remove(QChar(0)));

    //
    // Process Timers
    //
    if(timer_map.count("AUDs")==2) {  // ENCO-style AUD timer
      QList<int> aud_timers=timer_map.values("AUDs");
      double start=0.0;
      double end=0.0;
      if(aud_timers.at(0)<aud_timers.at(1)) {
	start=(double)aud_timers.at(0);
	end=(double)aud_timers.at(1);
      }
      else {
	start=(double)aud_timers.at(1);
	end=(double)aud_timers.at(0);
      }
      wave_data->setStartPos((int)(1000.0*start/((double)getSamplesPerSec())));
      wave_data->setEndPos((int)(1000.0*end/((double)getSamplesPerSec())));
    }
    else {
      for(int i=0;i<MAX_TIMERS;i++) {
	if(cart_timer_label[i]=="AUDs") {
	  wave_data->setStartPos((int)(1000.0*((double)cart_timer_sample[i])/
				       ((double)getSamplesPerSec())));
	}
	if(cart_timer_label[i]=="AUDe") {
	  wave_data->setEndPos((int)(1000.0*((double)cart_timer_sample[i])/
				     ((double)getSamplesPerSec())));
	}
      }
    }
    for(int i=0;i<MAX_TIMERS;i++) {
      if((cart_timer_label[i]=="SEGs")||(cart_timer_label[i]=="SEC1")) {
	wave_data->setSegueStartPos((int)(1000.0*((double)cart_timer_sample[i])/
					  ((double)getSamplesPerSec())));
      }
      if((cart_timer_label[i]=="SEGe")||(cart_timer_label[i]=="EOD ")) {
	wave_data->setSegueEndPos((int)(1000.0*((double)cart_timer_sample[i])/
					((double)getSamplesPerSec())));
      }
      if(cart_timer_label[i]=="INTs") {
	wave_data->setTalkStartPos((int)(1000.0*((double)cart_timer_sample[i])/
					 ((double)getSamplesPerSec())));
      }
      if(cart_timer_label[i]=="INTe") {
	wave_data->setTalkEndPos((int)(1000.0*((double)cart_timer_sample[i])/
				       ((double)getSamplesPerSec())));
      }
      if((cart_timer_label[i]=="INT ")||(cart_timer_label[i]=="INT1")) {
	wave_data->setTalkStartPos(0);
	wave_data->setTalkEndPos((int)(1000.0*((double)cart_timer_sample[i])/
				       ((double)getSamplesPerSec())));
      }
    }
  }
  return true;
}


bool RDWaveFile::GetBext(int fd)
{
  unsigned chunk_size;
  char *tag_buffer=NULL;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"bext",&chunk_size,bext_chunk_data,BEXT_CHUNK_SIZE)) {
    return false;
  }
  bext_chunk=true;

  bext_description=cutString((char *)bext_chunk_data,0,256);
  bext_originator=cutString((char *)bext_chunk_data,256,32);
  bext_originator_ref=cutString((char *)bext_chunk_data,288,32);
  bext_origination_date=cutDate((char *)bext_chunk_data,320);
  bext_origination_time=cutTime((char *)bext_chunk_data,330);
  bext_time_reference_low=bext_chunk_data[338]+256*bext_chunk_data[339]+
    65536*bext_chunk_data[340]+16777216*bext_chunk_data[341];
  bext_time_reference_high=bext_chunk_data[342]+256*bext_chunk_data[343]+
    65536*bext_chunk_data[344]+16777216*bext_chunk_data[345];
  bext_version=bext_chunk_data[346]+256*bext_chunk_data[347];
  for(int i=0;i<64;i++) {
    bext_umid[i]=bext_chunk_data[348+i];
  }

  //
  // Get the Coding History
  //
  if(chunk_size>602) {
    tag_buffer=(char *)malloc(chunk_size-602+1);
    CheckExitCode("RDWaveFile::GetBext()",
		  read(wave_file.handle(),tag_buffer,chunk_size-602));
    tag_buffer[chunk_size-602]=0;
    bext_coding_history=tag_buffer;
    free(tag_buffer);
    tag_buffer=NULL;
  }

  if(wave_data!=NULL) {
    wave_data->setMetadataFound(true);
    wave_data->setDescription(bext_description.remove(QChar(0)));
    wave_data->setOriginator(bext_originator.remove(QChar(0)));
    wave_data->setOriginatorReference(bext_originator_ref.remove(QChar(0)));
    wave_data->setOriginationDate(bext_origination_date);
    wave_data->setOriginationTime(bext_origination_time);
    wave_data->setCodingHistory(bext_coding_history.remove(QChar(0)));
  }
  return true;
}


bool RDWaveFile::GetMext(int fd)
{
  unsigned chunk_size;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"mext",&chunk_size,mext_chunk_data,MEXT_CHUNK_SIZE)) {
    return false;
  }
  mext_chunk=true;

  if((mext_chunk_data[0]&1)!=0) {
    mext_homogenous=true;
  }
  if((mext_chunk_data[0]&2)==0) {
    mext_padding_used=true;
  }
  if((mext_chunk_data[0]&4)!=0) {
    mext_rate_hacked=true;
  }
  if((mext_chunk_data[0]&8)!=0) {
    mext_free_format=true;
  }
  mext_frame_size=mext_chunk_data[2]+256*mext_chunk_data[3];
  mext_anc_length=mext_chunk_data[4]+256*mext_chunk_data[5];
  if((mext_chunk_data[6]&1)!=0) {
    mext_left_energy=true;
  }
  if((mext_chunk_data[6]&2)!=0) {
    mext_ancillary_private=true;
  }
  if((mext_chunk_data[6]&4)!=0) {
    mext_right_energy=true;
  }

  return true;
}


bool RDWaveFile::GetLevl(int fd)
{
  unsigned size=LEVL_CHUNK_SIZE;
  unsigned chunk_size;
  unsigned char frame[2];
  QDate date;
  QTime time;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"levl",&chunk_size,levl_chunk_data,LEVL_CHUNK_SIZE)) {
    return false;
  }
  levl_chunk=true;

  levl_version=ReadDword(levl_chunk_data,0);
  levl_format=ReadDword(levl_chunk_data,4);
  levl_points=ReadDword(levl_chunk_data,8);
  levl_block_size=ReadDword(levl_chunk_data,12);
  levl_channels=ReadDword(levl_chunk_data,16);
  levl_frames=ReadDword(levl_chunk_data,20);
  levl_peak_offset=ReadDword(levl_chunk_data,24);
  levl_block_offset=ReadDword(levl_chunk_data,28);
  levl_timestamp=QDateTime(cutDate((char *)levl_chunk_data,32),
			   cutTime((char *)levl_chunk_data,43));
  if(levl_block_size!=1152) {
    return true;
  }
  lseek(wave_file.handle(),FindChunk(wave_file.handle(),"levl",&size)+
	levl_block_offset-8,SEEK_SET);
  for(unsigned i=1;i<levl_frames;i++) {
    for(int j=0;j<levl_channels;j++) {
      CheckExitCode("RDWaveFile::GetLevl()",read(wave_file.handle(),frame,2));
      energy_data.push_back(frame[0]+256*frame[1]);
    }
  }
  if(levl_peak_offset==0xFFFFFFFF) {
    levl_peak_value=0;
  }
  else {
    levl_peak_value=energy_data[levl_peak_offset];
  }
  energy_loaded=true;
  has_energy=true;
  return true;
}


bool RDWaveFile::GetScot(int fd)
{
  unsigned chunk_size;
  int start_day;
  int start_year;
  int start_month;
  int start_hour;
  int end_day;
  int end_year;
  int end_month;
  int end_hour;
  unsigned cartnum;
  unsigned segue_start;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"scot",&chunk_size,scot_chunk_data,SCOT_CHUNK_SIZE)) {
    return false;
  }
  scot_chunk=true;
  scot_title=cutString((char *)scot_chunk_data,4,42);
  scot_artist=cutString((char *)scot_chunk_data,267,33);
  scot_etc=cutString((char *)scot_chunk_data,301,33);
  scot_year=cutString((char *)scot_chunk_data,338,4).toInt();
  scot_intro_length=cutString((char *)scot_chunk_data,335,2).toInt()*1000;
  start_year=cutString((char *)scot_chunk_data,69,2).toInt()+2000;
  start_month=cutString((char *)scot_chunk_data,65,2).toInt();
  start_day=cutString((char *)scot_chunk_data,67,2).toInt();
  cartnum=cutString((char *)scot_chunk_data,47,4).toUInt();
  segue_start=(0xFF&scot_chunk_data[88])+((0xFF&scot_chunk_data[89])<<8);
  if((start_month>0)&&(start_month<13)&&(start_day>0)&&(start_day<32)) {
    scot_start_date=QDate(start_year,start_month,start_day);
  }
  start_hour=cutString((char *)scot_chunk_data,77,2).toInt();
  if((start_hour>=129)&&(start_hour<=151)) {
    scot_start_time=QTime(start_hour-128,0,0);
  }
  end_year=cutString((char *)scot_chunk_data,75,2).toInt()+2000;
  end_month=cutString((char *)scot_chunk_data,71,2).toInt();
  end_day=cutString((char *)scot_chunk_data,73,2).toInt();
  if((end_month>0)&&(end_month<13)&&(end_day>0)&&(end_day<32)&&
    scot_start_date.isValid()) {
    scot_end_date=QDate(end_year,end_month,end_day);
  }
  else {
    scot_start_date=QDate();
    scot_end_date=QDate();
  }
  end_hour=cutString((char *)scot_chunk_data,78,2).toInt();
  if((end_hour>=129)&&(end_hour<=151)) {
    scot_end_time=QTime(end_hour-128,0,0);
  }
  if(wave_data!=NULL) {
    wave_data->setMetadataFound(true);
    wave_data->setTitle(scot_title.remove(QChar(0)).trimmed());
    wave_data->setArtist(scot_artist.remove(QChar(0)).trimmed());
    wave_data->setUserDefined(scot_etc.remove(QChar(0)).trimmed());
    wave_data->setReleaseYear(scot_year);
    wave_data->setCutId(QString::asprintf("%u",cartnum));
    wave_data->setTalkStartPos(0);
    wave_data->setTalkEndPos(scot_intro_length);
    if(segue_start>0) {
      wave_data->setSegueStartPos(getExtTimeLength()-10*segue_start);
      wave_data->setSegueEndPos(getExtTimeLength());
    }
    if(scot_start_date.isValid()) {
      wave_data->setStartDate(scot_start_date);
    }
    if(scot_start_time.isValid()) {
      wave_data->setStartTime(scot_start_time);
    }
    if(scot_end_date.isValid()) {
      wave_data->setEndDate(scot_end_date);
    }
    if(scot_end_time.isValid()) {
      wave_data->setEndTime(scot_end_time);
    }
  }
  return true;
}


bool RDWaveFile::GetAv10(int fd)
{
  //
  // The 'av10' chunk is used by BE AudioVault systems for metadata storage
  //
  unsigned chunk_size;
  QString str;
  int n;
  int pos;
  int offset;
  bool ok=false;
  int istate=0;
  QString label;
  QString arg;
  QString userdef;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"av10",&chunk_size,av10_chunk_data,AV10_CHUNK_SIZE)) {
    return false;
  }
  av10_chunk=true;

  //
  // Walk through the fields
  //
  for(unsigned i=2;i<chunk_size;i++) {
    switch(istate) {
      case 0:  // Label
	if(av10_chunk_data[i]==0) {
	  i++;
	  istate=1;
	}
	else {
	  label+=av10_chunk_data[i];
	}
	break;

      case 1:  // Argument
	if(av10_chunk_data[i]==0) {  // Found a label/value pair, see if we can
                                     // use it
	  if(label=="1") {  // Start/end markers
	    if((n=arg.indexOf(","))>0) {
	      pos=arg.left(n).toInt(&ok);
	      if(ok) {
		offset=arg.right(arg.length()-n-1).toInt(&ok);
		if(ok) {
		  if(wave_data!=NULL) {
		    wave_data->setStartPos(pos);
		    wave_data->setEndPos(pos+offset);
		    wave_data->setMetadataFound(true);
		  }
		}
	      }
	    }
	  }
	  if(label=="2") {  // Segue markers
	    if((n=arg.indexOf(","))>0) {
	      pos=arg.left(n).toInt(&ok);
	      if(ok) {
		offset=arg.right(arg.length()-n-1).toInt(&ok);
		if(ok) {
		  if(wave_data!=NULL) {
		    wave_data->setSegueStartPos(pos);
		    wave_data->setSegueEndPos(wave_data->endPos());
		    wave_data->setMetadataFound(true);
		  }
		}
	      }
	    }
	  }
	  if(label=="C") {  // AV Category
	    userdef+=("av_category="+arg+", ");
	  }
	  if(label=="CI") {  // AV Artist/Client
	    if(wave_data!=NULL) {
	      wave_data->setArtist(arg);
	    }
	  }
	  if(label=="CL") {  // AV Class
	    userdef+=("av_class="+arg+", ");
	  }
	  if(label=="CO") {  // AV Codes
	    userdef+=("av_codes="+arg+", ");
	  }
	  if(label=="D") {   // AV Description
	    if(wave_data!=NULL) {
	      wave_data->setTitle(arg);
	      wave_data->setMetadataFound(true);
	    }
	  }
	  if(label=="G") {   // AV ???
	  }
	  if(label=="IN") {  // AV Intro
	    if(ok) {
	      if(wave_data!=NULL) {
		wave_data->setTalkStartPos(wave_data->startPos());
		wave_data->setTalkEndPos(1000*arg.toInt());
		wave_data->setMetadataFound(true);
	      }
	    }
	  }
	  if(label=="K") {   // AV Kill Date
	  }
	  if(label=="N") {   // AV Name
	  }
	  if(label=="Q") {   // AV Outcue
	    if(wave_data!=NULL) {
	      wave_data->setOutCue(arg);
	      wave_data->setMetadataFound(true);
	    }
	  }
	  if(label=="S") {   // AV Start Date
	  }
	  if(label=="SR") {  // AV ???
	  }
	  // printf("Label: %s  Arg: %s\n",(const char *)label,(const char *)arg);
	  label="";
	  arg="";
	  istate=0;
	}
	else {
	  arg+=av10_chunk_data[i];
	}
	break;	
    }
    if(!userdef.isEmpty()) {
      if(wave_data!=NULL) {
	wave_data->setUserDefined(userdef.left(userdef.length()-2));
      }      
    }
  }

  return true;
}


bool RDWaveFile::GetAir1(int fd)
{
  //
  // The 'AIR1' chunk is used by AirForce systems for metadata storage
  //
  unsigned chunk_size;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"AIR1",&chunk_size,AIR1_chunk_data,AIR1_CHUNK_SIZE)) {
    return false;
  }
  AIR1_chunk_data[2047]=0;
  if(wave_data!=NULL) {
    wave_data->setTitle(cutString((char *)AIR1_chunk_data,0x102,27).
			trimmed().remove(QChar(0)));
    wave_data->setArtist(cutString((char *)AIR1_chunk_data,0x147,27).
			 trimmed().remove(QChar(0)));
    wave_data->setAlbum(cutString((char *)AIR1_chunk_data,0x163,27).
			trimmed().remove(QChar(0)));
    wave_data->setReleaseYear(cutString((char *)AIR1_chunk_data,0x17F,4).
			      toInt());
    wave_data->setMetadataFound(true);
  }
  AIR1_chunk=true;
  return true;
}


bool RDWaveFile::GetRdxl(int fd)
{
  off_t pos;
  unsigned chunk_size=0;
  char *chunk=NULL;

  if((pos=FindChunk(fd,"rdxl",&chunk_size))<0) {
    return false;
  }
  lseek(fd,SEEK_SET,pos);
  chunk=new char[chunk_size+1];
  memset(chunk,0,chunk_size+1);
  CheckExitCode("RDWaveFile::GetRdxl()",read(fd,chunk,chunk_size));
  rdxl_contents=QString::fromUtf8(chunk);
  delete chunk;

  if(wave_data!=NULL) {
    std::vector<RDWaveData> wavedatas;
    if(RDCart::readXml(&wavedatas,rdxl_contents)>1) {
      *wave_data=wavedatas[1];
    }
  }

  return true;
}


bool RDWaveFile::GetComm(int fd)
{
  unsigned chunk_size;

  /*
   * Load the chunk
   */
  if(!GetChunk(fd,"COMM",&chunk_size,comm_chunk_data,COMM_CHUNK_SIZE,true)) {
    comm_chunk=false;
    return false;
  }
  comm_chunk=true;


  format_tag=WAVE_FORMAT_PCM;   // We support only PCM AIFFs at the moment
  channels=comm_chunk_data[1]+256*fmt_chunk_data[0];
  sample_length=comm_chunk_data[5]+256*comm_chunk_data[4]+
    65536*comm_chunk_data[3]+16777216*comm_chunk_data[2];
  bits_per_sample=comm_chunk_data[6]+256*comm_chunk_data[7];
  samples_per_sec=256*(0xFF&comm_chunk_data[10])+(0xFF&comm_chunk_data[11]);

  return true;
}


bool RDWaveFile::GetList(int fd)
{
  unsigned chunk_size=0;
  if((wave_data==NULL)||(FindChunk(fd,"list",&chunk_size)<0)) {
    return false;
  }
  unsigned char *chunk_data=new unsigned char[chunk_size];
  CheckExitCode("RDWaveFile::GetList()",read(fd,chunk_data,chunk_size));
  unsigned offset=4;
  while(ReadListElement(chunk_data,&offset,chunk_size));
  if((wave_data->segueStartPos()>=0)&&(wave_data->segueEndPos()<0)) {
    wave_data->setSegueEndPos(ext_time_length);
  }
  return true;
}


bool RDWaveFile::ReadListElement(unsigned char *buffer,unsigned *offset,
				unsigned size)
{
  if(*offset>=size) {
    return false;
  }
  char tag[5];
  for(int i=0;i<4;i++) {
    tag[i]=buffer[*offset+i];
  }
  tag[4]=0;
  *offset+=4;
  unsigned len=ReadDword(buffer,*offset);
  *offset+=4;
  if(!strcmp(tag,"tref")) {
    wave_data->setTmciSongId(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tttl")) {
    wave_data->setTitle(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tart")) {
    wave_data->setArtist(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }

  if(!strcmp(tag,"tcom")) {
    wave_data->setComposer(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tpub")) {
    wave_data->setPublisher(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tlic")) {
    wave_data->setLicensingOrganization(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tlab")) {
    wave_data->setLabel(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tint")) {
    wave_data->setTalkStartPos(0);
    wave_data->setTalkEndPos(RDSetTimeLength(((char *)buffer)+(*offset)));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"ttim")) {
    wave_data->setStartPos(0);
    wave_data->setEndPos(RDSetTimeLength(((char *)buffer)+(*offset)));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tend")) {
    wave_data->setEndType((RDWaveData::EndType)(((char *)buffer)+(*offset))[0]);
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tyr ")) {
    wave_data->setReleaseYear(QString(((char *)buffer)+(*offset)).toInt());
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"taux")) {
    wave_data->setSegueStartPos(RDSetTimeLength(((char *)buffer)+(*offset)));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tbpm")) {
    wave_data->setBeatsPerMinute(QString(((char *)buffer)+(*offset)).toInt());
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"talb")) {
    wave_data->setAlbum(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tpli")) {
    wave_data->setCopyrightNotice(((char *)buffer)+(*offset));
    wave_data->setMetadataFound(true);
  }
  if(!strcmp(tag,"tisr")) {
    wave_data->setIsrc(QString(((char *)buffer)+(*offset)).remove(" "));
    wave_data->setMetadataFound(true);
  }

  *offset+=len;
  while(((*offset)<size) && (buffer[*offset]==0)) {
    (*offset)++;
  }
  return true;
}


bool RDWaveFile::ReadTmcMetadata(int fd)
{
  char buffer[256];
  QString current_tag;

  lseek(fd,data_length+4,SEEK_SET);
  while(GetLine(fd,buffer,255)) {
    if(buffer[0]=='#') {
      current_tag=QString(buffer+1);
    }
    else {
      if(!current_tag.isEmpty()) {
	ReadTmcTag(current_tag,QString(buffer));
      }
    }
  }
  return false;
}


void RDWaveFile::ReadTmcTag(const QString tag,const QString value)
{
  if(tag=="TITLE") {
    wave_data->setTitle(value.trimmed().remove(QChar(0)));
    wave_data->setMetadataFound(true);
  }
  if(tag=="ARTIST") {
    wave_data->setArtist(value.trimmed().remove(QChar(0)));
    wave_data->setMetadataFound(true);
  }
  if(tag=="COMPOSER") {
    wave_data->setComposer(value.trimmed().remove(QChar(0)));
    wave_data->setMetadataFound(true);
  }
  if(tag=="PUBLISHER") {
    wave_data->setPublisher(value.trimmed().remove(QChar(0)));
    wave_data->setMetadataFound(true);
  }
  if(tag=="LICENSE") {
    wave_data->setLicensingOrganization(value.trimmed().remove(QChar(0)));
    wave_data->setMetadataFound(true);
  }
  if(tag=="LABEL") {
    wave_data->setLabel(value.trimmed().remove(QChar(0)));
    wave_data->setMetadataFound(true);
  }
  if(tag=="ALBUM") {
    wave_data->setAlbum(value.trimmed().remove(QChar(0)));
    wave_data->setMetadataFound(true);
  }
  if(tag=="YEAR") {
    wave_data->setReleaseYear(value.trimmed().remove(QChar(0)).toInt());
    wave_data->setMetadataFound(true);
  }
  if(tag=="INTRO") {
    wave_data->setTalkStartPos(0);
    wave_data->setTalkEndPos(RDSetTimeLength(value.trimmed().remove(QChar(0))));
    wave_data->setMetadataFound(true);
  }
  if(tag=="AUX") {
    wave_data->setSegueStartPos(RDSetTimeLength(value.trimmed().remove(QChar(0))));
    wave_data->setMetadataFound(true);
  }
  if(tag=="END") {
    wave_data->setEndType((RDWaveData::EndType)((char)value.at(0).cell()));
    wave_data->setMetadataFound(true);
  }
  if(tag=="TMCIREF") {
    wave_data->setTmciSongId(value.trimmed().remove(QChar(0)));
    wave_data->setMetadataFound(true);
  }
  if(tag=="BPM") {
    wave_data->setBeatsPerMinute(value.toInt());
    wave_data->setMetadataFound(true);
  }
  if(tag=="ISRC") {
    QString str=value;
    wave_data->setIsrc(str.remove(" ").trimmed().remove(QChar(0)));
    wave_data->setMetadataFound(true);
  }
  if(tag=="PLINE") {
    wave_data->setCopyrightNotice(value.trimmed().remove(QChar(0)));
    wave_data->setMetadataFound(true);
  }
}


bool RDWaveFile::GetLine(int fd,char *buffer,int max_len)
{
  for(int i=0;i<max_len;i++) {
    if(read(fd,buffer+i,1)==0) {
      return false;
    }
    if(buffer[i]==10) {
      if(buffer[i-1]==13) {
	buffer[i-1]=0;
      }
      else {
	buffer[i]=0;
      }
      return true;
    }
  }
  buffer[max_len-1]=0;
  return true;
}


void RDWaveFile::ReadId3Metadata()
{
  //
  // This whole method is a mess!
  //
  // The short story: Rivendell versions 1.x and 2.x (as well as a whole
  // slew of other audio applications) used the ID3Lib library
  // [http://id3lib.sourceforge.net/api/index.html] to read and
  // write ID3 tags. As it turns out, that library does not properly
  // implement the ID3v2 standard for using UTF-8 characters in tags,
  // resulting in tags that the TagLib library (which does understand
  // UTF-8 properly) may refuse to read. This method attempts to be decently
  // reliable at detecting at least the 'RDXL' tag used by this and previous
  // versions of Rivendell to provide a 'full-fidelity' transfer of metadata
  // between Rivendell databases. The basic methodology is: use TagLib
  // if possible, but if that fails, fall back to ID3Lib and try again.
  //

  if(wave_data==NULL) {
    return;
  }
  bool using_rdxl=false;
  TagLib::FileRef tagref(wave_file_name.toUtf8());
  if(tagref.file()==NULL) {  // Take another look with ID3Lib
    ID3_Frame *frame=NULL;
    ID3_Tag id3_tag(wave_file_name.toUtf8());
    if((frame=id3_tag.Find(ID3FID_USERTEXT,ID3FN_DESCRIPTION,"rdxl"))!=NULL) {
      rdxl_contents=ID3_GetString(frame,ID3FN_TEXT);
      if(wave_data!=NULL) {
	std::vector<RDWaveData> wavedatas;
	if(RDCart::readXml(&wavedatas,rdxl_contents)>1) {
	  *wave_data=wavedatas[1];
	  using_rdxl=true;
	}
      }
    }
    return;
  }
  TagLib::PropertyMap tags=tagref.file()->properties();

  //
  // Check for a mangled RDXL frame
  // If we find one, use id3lib to process it, otherwise continue with TagLib
  //
  for(TagLib::PropertyMap::ConstIterator it=tags.begin();it!=tags.end();++it) {
    QString name=QString::fromUtf8(it->first.toCString(true));
    if(name==QString::fromUtf8("牤硬")) {  // Mangled RD v2.x RDXL Frame
      using_rdxl=true;
      ID3_Frame *frame=NULL;
      ID3_Tag id3_tag(wave_file_name.toUtf8());
      if((frame=id3_tag.Find(ID3FID_USERTEXT,ID3FN_DESCRIPTION,"rdxl"))!=NULL) {
	rdxl_contents=ID3_GetString(frame,ID3FN_TEXT);
	if(wave_data!=NULL) {
	  std::vector<RDWaveData> wavedatas;
	  if(RDCart::readXml(&wavedatas,rdxl_contents)>1) {
	    *wave_data=wavedatas[1];
	  }
	}
      }
    }
    if(name=="RDXL") {
      using_rdxl=true;
      rdxl_contents=QString::fromUtf8(it->second.begin()->toCString(true));
      if(wave_data!=NULL) {
	std::vector<RDWaveData> wavedatas;
	if(RDCart::readXml(&wavedatas,rdxl_contents)>1) {
	  *wave_data=wavedatas[1];
	}
      }
    }
  }

  if(!using_rdxl) {
    for(TagLib::PropertyMap::ConstIterator it=tags.begin();
	it!=tags.end();++it) {
      QString name=QString::fromUtf8(it->first.toCString(true)).toUpper();
      if(((name=="TITLE")||(name=="TIT2"))&&(it->second.size()!=0)) {
	wave_data->
	  setTitle(QString::fromUtf8(it->second.begin()->toCString(true)));
	wave_data->setMetadataFound(true);
      }
      if((name=="ARTIST")&&(it->second.size()!=0)) {
	wave_data->
	  setArtist(QString::fromUtf8(it->second.begin()->toCString(true)));
	wave_data->setMetadataFound(true);
      }
      if(((name=="ALBUM")||(name=="TALB"))&&(it->second.size()!=0)) {
	wave_data->
	  setAlbum(QString::fromUtf8(it->second.begin()->toCString(true)));
	wave_data->setMetadataFound(true);
      }
      if(((name=="LABEL")||(name=="TPUB"))&&(it->second.size()!=0)) {
	wave_data->
	  setLabel(QString::fromUtf8(it->second.begin()->toCString(true)));
	wave_data->setMetadataFound(true);
      }
      if(((name=="COMPOSER")||(name=="TCOM"))&&(it->second.size()!=0)) {
	wave_data->
	  setComposer(QString::fromUtf8(it->second.begin()->toCString(true)));
	wave_data->setMetadataFound(true);
      }
      if(((name=="CONDUCTOR")||(name=="TPE3"))&&(it->second.size()!=0)) {
	wave_data->
	  setConductor(QString::fromUtf8(it->second.begin()->toCString(true)));
	wave_data->setMetadataFound(true);
      }
      if((name=="PUBLISHER")&&(it->second.size()!=0)) {
	wave_data->
	  setPublisher(QString::fromUtf8(it->second.begin()->toCString(true)));
	wave_data->setMetadataFound(true);
      }
      if((name=="ISRC")&&(it->second.size()!=0)) {
	wave_data->
	  setIsrc(QString::fromUtf8(it->second.begin()->toCString(true)));
	wave_data->setMetadataFound(true);
      }
      if((name=="COPYRIGHT")&&(it->second.size()!=0)) {
	wave_data->
	  setCopyrightNotice(QString::fromUtf8(it->second.begin()->
					       toCString(true)));
	wave_data->setMetadataFound(true);
      }
      if(((name=="YEAR")||(name=="TYER")||(name=="DATE"))&&
	 (it->second.size()!=0)) {
	QStringList f0=QString(it->second.begin()->toCString(true)).split("-");
	if((f0.size()>=1)&&(f0.at(0).length()==4)) {
	  bool ok=false;
	  int year=f0.at(0).toInt(&ok);
	  if(ok&&(year>0)) {
	    wave_data->setReleaseYear(year);
	    wave_data->setMetadataFound(true);
	  }
	}
      }
      if(((name=="BPM")||(name=="TBPM"))&&(it->second.size()!=0)) {
	QStringList f0=QString(it->second.begin()->toCString(true)).split(".");
	if(f0.size()>=1) {
	  bool ok=false;
	  int beats=f0.at(0).toInt(&ok);
	  if(ok&&(beats>0)) {
	    wave_data->setBeatsPerMinute(beats);
	    wave_data->setMetadataFound(true);
	  }
	}
      }
      /*
      printf("TAG: %s = ",it->first.toCString(true));
      for(TagLib::StringList::ConstIterator it2=it->second.begin();it2!=
	    it->second.end();
	  ++it2) {
	printf("%s ",it2->toCString(true));
      }
      printf("\n");
      */
    }
  }
}


bool RDWaveFile::GetMpegHeader(int fd,int offset)
{
  //
  // See:
  //
  //   http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm
  //   https://www.codeproject.com/Articles/8295/MPEG-Audio-Frame-Header
  //
  // for helpful information regarding the arcana of interpreting MPEG
  // header data structures. Sometimes, fasting and prayer can help too!
  //

  //
  // Bitrate table
  //
  // __bitrates[VERSION_INDEX][LAYER_INDEX][BITRATE_INDEX]
  //
  // N.B. Bitrate values in this table are in thousands of bits per second.
  //      Certain other data structures in this class require bits per second!
  //
  static int __bitrates[4][4][16]={
    {  // *** MPEG 2.5 ***
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},       // Reserved Layer
      {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,-1},     // Layer III
      {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,-1},     // Layer II
      {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,-1} // Layer I
    },

    {  // *** Invalid MPEG Version ***
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},       // Reserved Layer
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},       // Layer III
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},       // Layer II
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}        // Layer I
    },

    {  // *** MPEG 2 ***
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},       // Reserved Layer
      {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,-1},     // Layer III
      {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,-1},     // Layer II
      {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,-1} // Layer I
    },

    {  // *** MPEG 1 ***
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},       // Reserved Layer
      {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,-1}, // Layer III
      {0,32,48,56,64,80,96,112,128,160,192,224,256,320,384,-1},// Layer II
      {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,-1} // Layer I
    }
  };

  //
  // Sample Rate Table
  //
  // __samplerates[VERSION_INDEX][SAMPRATE_INDEX]
  static int __samplerates[4][4]={
    {11025,12000,8000,-1},  // *** MPEG 2.5 ***
    {-1,-1,-1,-1},          // *** Invalid MPEG Version ***
    {22050,24000,16000,-1}, // *** MPEG 2 ***
    {44100,48000,32000,-1}  // *** MPEG 1 ***
  };

  //
  // Channels table
  //
  // __channels[MODE_INDEX]
  static int __channels[4]={2,2,2,1};

  //
  // Head Mode Table
  //
  // __head_modes[MODE_INDEX];
  static int __head_modes[4]={ACM_MPEG_STEREO,ACM_MPEG_JOINTSTEREO,
			      ACM_MPEG_DUALCHANNEL,ACM_MPEG_SINGLECHANNEL};

  //
  // Layer Numbers Table
  //
  // __layer_numbers[LAYER_INDEX]
  static int __layer_numbers[4]={0,3,2,1};

  //
  // Samples per MPEG Frame Table
  //
  // __samples_per_frames[VERSION_INDEX][LAYER_INDEX]
  static int __samples_per_frame[4][4]={
    {0,576,1152,384},  // *** MPEG 2.5 ***
    {0,0,0,384},  // *** Invalid MPEG Version ***
    {0,576,1152,384},  // *** MPEG 2 ***
    {0,1152,1152,384}   // *** MPEG 1 ***
  };

  //
  // Side Data Offset Table (Layer III only)
  //
  // __side_data_offset[VERSION_INDEX][MODE]
  static int __side_data_offset[4][4]={
    {17,17,17,9},  // *** MPEG 2.5 ***
    {0,0,0,0},     // *** Invalid MPEG Version ***
    {17,17,17,9},  // *** MPEG 2 ***
    {32,32,32,17}  // *** MPEG 1 ***
  };

  char header[4];
  char *frame=NULL;
  ssize_t n;
  // off_t frame_start;
  int version_index;
  int layer_index;
  int bitrate_index;
  int samprate_index;
  int padding=0;
  int mode_index;
  int frame_size;
  int total_frame_quan=-1;

  lseek(fd,offset,SEEK_SET);
  if((n=read(fd,header,4))!=4) {
    return false;
  }
  //  frame_start=lseek(fd,0,SEEK_CUR)-4;

  //
  // Sync bits
  //
  if(((0xFF&header[0])!=0xFF)||((0xE0&header[1])!=0xE0)) {
    return false;
  }

  //
  // MPEG Audio Version ID
  //
  version_index=(0x18&header[1])>>3;
  if(version_index==0x01) {  // Illegal Version ID
    return false;
  }

  //
  // Layer
  //
  layer_index=(0x06&header[1])>>1;
  if(layer_index==0x00) {  // Illegal Layer
    return false;
  }
  head_layer=__layer_numbers[layer_index];

  //
  // Bitrate
  //
  bitrate_index=(0xF0&header[2])>>4;
  if(__bitrates[version_index][layer_index][bitrate_index]<0) {
    return false;
  }
  head_bit_rate=1000*__bitrates[version_index][layer_index][bitrate_index];

  //
  // Samplerate
  //
  samprate_index=(0x0C&header[2])>>2;
  if((__samplerates[version_index][samprate_index]<0)&&(bitrate_index!=0)) {
    return false;
  }
  samples_per_sec=__samplerates[version_index][samprate_index];

  //
  // Padding bit
  //
  padding=(0x02&header[2])>>1;

  //
  // Mode
  //
  mode_index=(0xC0&header[3])>>6;
  head_mode=__head_modes[mode_index];
  channels=__channels[mode_index];

  //
  // Flags
  //
  head_flags=0;
  if((0x01&header[2])!=0) {
    head_flags|=ACM_MPEG_PRIVATEBIT;
  }    
  if((header[3]&0x08)!=0) {
    head_flags|=ACM_MPEG_COPYRIGHT;
  }
  if((header[3]&0x04)!=0) {
    head_flags|=ACM_MPEG_ORIGINALHOME;
  }
  if(version_index==3) {
    head_flags|=ACM_MPEG_ID_MPEG1;
  }

  //
  // Frame Size
  //
  if(layer_index==3) {  // Layer I
    frame_size=(12000*__bitrates[version_index][layer_index][bitrate_index]/
		__samplerates[version_index][samprate_index]+padding)*4;
  }
  else {  // Layers II and III
    frame_size=(144000*__bitrates[version_index][layer_index][bitrate_index]/
		__samplerates[version_index][samprate_index])+padding;
  }
  
  //
  // Load the frame data
  //
  frame=new char[frame_size];
  if((n=read(fd,frame,frame_size-4))!=(frame_size-4)) {
    delete frame;
    return false;
  }

  //
  // Look for the Xing/Info tag (for VBR data)
  //
  if(((frame[__side_data_offset[version_index][mode_index]]=='X')&&
      (frame[__side_data_offset[version_index][mode_index]+1]=='i')&&
      (frame[__side_data_offset[version_index][mode_index]+2]=='n')&&
      (frame[__side_data_offset[version_index][mode_index]+3]=='g'))||
     ((frame[__side_data_offset[version_index][mode_index]]=='I')&&
      (frame[__side_data_offset[version_index][mode_index]+1]=='n')&&
      (frame[__side_data_offset[version_index][mode_index]+2]=='f')&&
      (frame[__side_data_offset[version_index][mode_index]+3]=='o'))) {
    if((frame[__side_data_offset[version_index][mode_index]+7]&0x01)==0x01) {
      total_frame_quan=
	16777216*frame[__side_data_offset[version_index][mode_index]+8]+
	65536*frame[__side_data_offset[version_index][mode_index]+9]+
	256*frame[__side_data_offset[version_index][mode_index]+10]+
	frame[__side_data_offset[version_index][mode_index]+11];
      time_length=
	total_frame_quan*__samples_per_frame[version_index][layer_index]/
	__samplerates[version_index][samprate_index];
      ext_time_length=1000*(int64_t)total_frame_quan*
	(int64_t)__samples_per_frame[version_index][layer_index]/
	(int64_t)__samplerates[version_index][samprate_index];
    }
  }
  if(total_frame_quan<0) {
    //
    // No VBR tag, assume CBR
    //
    sample_length=1152.0*((double)data_length/(144.0*(double)head_bit_rate/
					       (double)samples_per_sec));
    ext_time_length=1000.0*(double)sample_length/(double)samples_per_sec;
    time_length=ext_time_length/1000;
  }
  mpeg_frame_size=144*head_bit_rate/samples_per_sec;

  delete frame;

  return true;
}


int RDWaveFile::GetAtxOffset(int fd)
{
  unsigned char buffer[MAX_ATX_HEADER_SIZE];
  lseek(fd,0,SEEK_SET);

  int n=read(fd,buffer,MAX_ATX_HEADER_SIZE-1);
  for(int i=0;i<n;i++) {
    if(buffer[i]==0xFF) {
      return i;
    }
  }
  return -1;
}


bool RDWaveFile::GetFlacStreamInfo()
{
#if HAVE_FLAC
  FLAC__StreamMetadata sinfo;
  if(!FLAC__metadata_get_streaminfo(wave_file_name.toUtf8(),&sinfo)) {
    return false;
  }
  samples_per_sec=sinfo.data.stream_info.sample_rate;
  bits_per_sample=sinfo.data.stream_info.bits_per_sample;
  sample_length=sinfo.data.stream_info.total_samples;
  channels=sinfo.data.stream_info.channels;
  ext_time_length=(unsigned)(1000.0*(double)sample_length/(double)samples_per_sec);
  time_length=ext_time_length/1000;
  return true;
#else
  return false;
#endif  // HAVE_FLAC
}


void RDWaveFile::ReadFlacMetadata()
{
#ifdef HAVE_FLAC_METADATA
  QString artist;
  QString composer;
  FLAC__StreamMetadata* tags;
  if(!FLAC__metadata_get_tags(wave_file_name.toUtf8(),&tags)) {
    return;
  }
  for(unsigned iCommentIndex=0;
      iCommentIndex<tags->data.vorbis_comment.num_comments;
      ++iCommentIndex)
  {
    FLAC__StreamMetadata_VorbisComment_Entry&
      comment(tags->data.vorbis_comment.comments[iCommentIndex]);
	   
    // comment.entry is a UTF-8 string of comment.length octets 
    // (http://www.xiph.org/vorbis/doc/v-comment.html)
    QString entry=
      QString::fromUtf8((const char *)comment.entry, comment.length);
	   
    // every entry is a name=value pair. Name is not allowed to contain a '='
    // so we just scan to the first instance of it.
    int nameLength = entry.indexOf('=');
    if(nameLength < 0) continue; // malformed comment, it would seem
    QString name = entry.left(nameLength), value = entry.mid(nameLength + 1);
	   
    if(name=="TITLE") { 
      wave_data->setTitle(value); 
      wave_data->setMetadataFound(true); 
      continue;
    }
    if(name=="ARTIST") { 
      composer=value;
      wave_data->setMetadataFound(true);
      continue;
    }
    if(name=="PERFORMER") { 
      artist=value;
      wave_data->setMetadataFound(true);
    }
    if(name=="ALBUM") { 
      wave_data->setAlbum(value);
      wave_data->setMetadataFound(true);
      continue;
    }
    if(name=="ORGANIZATION") {
      wave_data->setLabel(value);
      wave_data->setMetadataFound(true);
      continue;
    }
    // TODO: Parse the date field to get the year out
    //if(name == "DATE") { wave_data->setReleaseYear(value);
    //wave_data->setMetadataFound(true); continue; }
    if(name=="ISRC") { 
      wave_data->setIsrc(value); 
      wave_data->setMetadataFound(true); 
      continue; 
    }
  }
  if((!artist.isEmpty())&&(!composer.isEmpty())) {
    wave_data->setArtist(artist);
    wave_data->setComposer(composer);
  }
  else {
    if(!artist.isEmpty()) {
      wave_data->setArtist(artist);
    }
    else {
      wave_data->setArtist(composer);
    }
  }
  FLAC__metadata_object_delete(tags);
#endif  // HAVE_FLAC_METADATA
}


bool RDWaveFile::MakeFmt()
{
  if((format_tag!=WAVE_FORMAT_PCM)&&(format_tag!=WAVE_FORMAT_MPEG)) {
    return false;
  }
  if((channels!=1)&&(channels!=2)) {
    return false;
  }
  if(samples_per_sec==0) {
    return false;
  }
  if(format_tag==WAVE_FORMAT_PCM) {
    switch(bits_per_sample) {
    case 8:
      block_align=channels;
      cart_level_ref=0x80;
	break;

    case 16:
      block_align=2*channels;
      cart_level_ref=0x8000;
      break;

    case 24:
      block_align=3*channels;
      cart_level_ref=0x800000;
      break;

    case 32:
      block_align=4*channels;
      cart_level_ref=0x80000000;
      break;

    default:
      return false;
      break;
    }
    avg_bytes_per_sec=block_align*samples_per_sec;
    cb_size=0;
    WriteSword(fmt_chunk_data,0,format_tag);
    WriteSword(fmt_chunk_data,2,channels);
    WriteDword(fmt_chunk_data,4,samples_per_sec);
    WriteDword(fmt_chunk_data,8,avg_bytes_per_sec);
    WriteSword(fmt_chunk_data,12,block_align);
    WriteSword(fmt_chunk_data,14,bits_per_sample);
    WriteSword(fmt_chunk_data,16,cb_size);
    fmt_size=18;
    return true;
  }
  if(format_tag==WAVE_FORMAT_MPEG) {
    bits_per_sample=0;
    block_align=144*head_bit_rate/samples_per_sec;
    cb_size=40;
    if(head_layer==0) {
      return false;
    }
    if(head_bit_rate==0) {
      return false;
    }
    if((!mext_padding_used)&&((samples_per_sec==11025)||
			      (samples_per_sec==22050)||
			      (samples_per_sec==44100))) {
      avg_bytes_per_sec=samples_per_sec*block_align/1152;
    }
    else {
      avg_bytes_per_sec=head_bit_rate/8;
    }
    if(head_mode==0) {
      return false;
    }
    head_flags|=ACM_MPEG_ID_MPEG1;

    WriteSword(fmt_chunk_data,0,format_tag);
    WriteSword(fmt_chunk_data,2,channels);
    WriteDword(fmt_chunk_data,4,samples_per_sec);
    WriteDword(fmt_chunk_data,8,avg_bytes_per_sec);
    WriteSword(fmt_chunk_data,12,block_align);
    WriteSword(fmt_chunk_data,14,bits_per_sample);
    WriteSword(fmt_chunk_data,16,cb_size);
    WriteSword(fmt_chunk_data,18,head_layer);
    WriteDword(fmt_chunk_data,20,head_bit_rate);
    WriteSword(fmt_chunk_data,24,head_mode);
    WriteSword(fmt_chunk_data,26,head_mode_ext);
    WriteSword(fmt_chunk_data,28,head_emphasis);
    WriteSword(fmt_chunk_data,30,head_flags);
    WriteDword(fmt_chunk_data,32,0);
    WriteDword(fmt_chunk_data,36,0);
    fmt_size=40;
    return true;
  }    
  return false;
}


bool RDWaveFile::MakeCart(unsigned ptr_offset)
{
  for(int i=0;i<CART_CHUNK_SIZE;i++) {
    cart_chunk_data[i]=0;
  }
  sprintf((char *)cart_chunk_data,"%4s",
	  CART_VERSION);
  if(!cart_title.isEmpty()) {
    sprintf((char *)cart_chunk_data+4,"%s",
	    cart_title.left(64).toUtf8().constData());
  }
  if(!cart_artist.isEmpty()) {
    sprintf((char *)cart_chunk_data+68,"%s",
	    cart_artist.left(64).toUtf8().constData());
  }
  if(!cart_cut_id.isEmpty()) {
    sprintf((char *)cart_chunk_data+132,"%s",
	    cart_cut_id.left(64).toUtf8().constData());
  }
  if(!cart_client_id.isEmpty()) {
    sprintf((char *)cart_chunk_data+196,"%s",
	    cart_client_id.left(64).toUtf8().constData());
  }
  if(!cart_category.isEmpty()) {
    sprintf((char *)cart_chunk_data+260,"%s",
	    cart_category.left(64).toUtf8().constData());
  }
  if(!cart_classification.isEmpty()) {
    sprintf((char *)cart_chunk_data+324,"%s",
	    cart_classification.left(64).toUtf8().constData());
  }
  if(!cart_out_cue.isEmpty()) {
    sprintf((char *)cart_chunk_data+388,"%s",
	    cart_out_cue.left(64).toUtf8().constData());
  }
  if(cart_start_date.isValid()) {
    sprintf((char *)cart_chunk_data+452,"%04d-%02d-%02d",
	    cart_start_date.year(),
	    cart_start_date.month(),
	    cart_start_date.day());
  }
  else {
    sprintf((char *)cart_chunk_data+452,"1900-01-01");
  }
  if(cart_start_time.isValid()) {
    sprintf((char *)cart_chunk_data+462,"%02d:%02d:%02d",
	    cart_start_time.hour(),
	    cart_start_time.minute(),
	    cart_start_time.second());
  }
  else {
    sprintf((char *)cart_chunk_data+462,"00:00:00");
  }
  if(cart_end_date.isValid()) {
    sprintf((char *)cart_chunk_data+470,"%04d-%02d-%02d",
	    cart_end_date.year(),
	    cart_end_date.month(),
	    cart_end_date.day());
  }
  else {
    sprintf((char *)cart_chunk_data+470,"9999-12-31");
  }
  if(cart_end_time.isValid()) {
    sprintf((char *)cart_chunk_data+480,"%02d:%02d:%02d",
	    cart_end_time.hour(),
	    cart_end_time.minute(),
	    cart_end_time.second());
  }
  else {
    sprintf((char *)cart_chunk_data+480,"23:59:59");
  }
  snprintf((char *)cart_chunk_data+488,64,"%s",PACKAGE);
  snprintf((char *)cart_chunk_data+552,64,"%s",VERSION);
  if(!cart_user_def.isEmpty()) {
    sprintf((char *)cart_chunk_data+616,"%s",
	    cart_user_def.left(64).toUtf8().constData());
  }
  WriteDword(cart_chunk_data,680,cart_level_ref);
  if(wave_data!=NULL) {
    int timer=0;
    if((wave_data->segueStartPos()>=0)&&
       (wave_data->segueEndPos()>wave_data->segueStartPos())) {
      sprintf((char *)cart_chunk_data+684+timer*MAX_TIMERS,"SEGs");
      WriteDword(cart_chunk_data,688+timer*MAX_TIMERS,
		 FrameOffset(wave_data->segueStartPos()-ptr_offset_msecs));
      timer++;
      sprintf((char *)cart_chunk_data+684+timer*MAX_TIMERS,"SEGe");
      WriteDword(cart_chunk_data,688+timer*MAX_TIMERS,
		 FrameOffset(wave_data->segueEndPos()-ptr_offset_msecs));
      timer++;
    }
    if((wave_data->talkStartPos()>=0)&&
       (wave_data->talkEndPos()>wave_data->talkStartPos())) {
      sprintf((char *)cart_chunk_data+684+timer*MAX_TIMERS,"INTs");
      WriteDword(cart_chunk_data,688+timer*MAX_TIMERS,
		 FrameOffset(wave_data->talkStartPos()-ptr_offset_msecs));
      timer++;
      sprintf((char *)cart_chunk_data+684+timer*MAX_TIMERS,"INTe");
      WriteDword(cart_chunk_data,688+timer*MAX_TIMERS,
		 FrameOffset(wave_data->talkEndPos()-ptr_offset_msecs));
      timer++;
    }
    if((wave_data->startPos()>=0)&&
       (wave_data->endPos()>wave_data->startPos())) {
      sprintf((char *)cart_chunk_data+684+timer*MAX_TIMERS,"AUDs");
      WriteDword(cart_chunk_data,688+timer*MAX_TIMERS,
		 FrameOffset(wave_data->startPos()-ptr_offset_msecs));
      timer++;
      sprintf((char *)cart_chunk_data+684+timer*MAX_TIMERS,"AUDe");
      WriteDword(cart_chunk_data,688+timer*MAX_TIMERS,
		 FrameOffset(wave_data->endPos()-ptr_offset_msecs));
      timer++;
    }
  }
  if(!cart_url.isEmpty()) {
    sprintf((char *)cart_chunk_data+1020,"%s",
	    cart_url.left(1024).toUtf8().constData());
  }
  return true;
}


bool RDWaveFile::MakeBext()
{
  bext_coding_size=bext_coding_history.length()+BEXT_CHUNK_SIZE;
  bext_coding_data=(unsigned char *)realloc(bext_coding_data,bext_coding_size);

  for(int i=0;i<BEXT_CHUNK_SIZE;i++) {
    bext_coding_data[i]=0;
  }
  if(!bext_description.isEmpty()) {
    sprintf((char *)bext_coding_data,"%s",
	    bext_description.left(256).toUtf8().constData());
  }
  if(!bext_originator.isEmpty()) {
    sprintf((char *)bext_coding_data+256,"%s",
	    bext_originator.left(32).toUtf8().constData());
  }
  if(!bext_originator_ref.isEmpty()) {
    sprintf((char *)bext_coding_data+288,"%s",
	    bext_originator_ref.left(32).toUtf8().constData());
  }
  sprintf((char *)bext_coding_data+320,"%04d-%02d-%02d",
	  bext_origination_date.year(),
	  bext_origination_date.month(),
	  bext_origination_date.day());
  sprintf((char *)bext_coding_data+330,"%02d:%02d:%02d",
	  bext_origination_time.hour(),
	  bext_origination_time.minute(),
	  bext_origination_time.second());
  WriteDword(bext_coding_data,338,bext_time_reference_low);
  WriteDword(bext_coding_data,342,bext_time_reference_high);
  WriteSword(bext_coding_data,346,BWF_VERSION);
  for(int i=0;i<64;i++) {
    bext_coding_data[i+348]=bext_umid[i];
  }
  if(!bext_coding_history.isEmpty()) {
    sprintf((char *)bext_coding_data+602,"%s",
	    bext_coding_history.toUtf8().constData());
  }
  return true;
}


bool RDWaveFile::MakeMext()
{
  for(int i=0;i<MEXT_CHUNK_SIZE;i++) {
    mext_chunk_data[i]=0;
  }
  if(mext_homogenous) {
    mext_chunk_data[0]|=1;
  }
  if(!mext_padding_used) {
    mext_chunk_data[0]|=2;
  }
  if(mext_rate_hacked) {
    mext_chunk_data[0]|=4;
  }
  if(mext_free_format) {
    mext_chunk_data[0]|=8;
  }
  if(mext_homogenous) {
    WriteSword(mext_chunk_data,2,mext_frame_size);
  }
  WriteSword(mext_chunk_data,4,mext_anc_length);
  if(mext_left_energy) {
    mext_chunk_data[6]|=1;
  }
  if(mext_ancillary_private) {
    mext_chunk_data[6]|=2;
  }
  if(mext_right_energy) {
    mext_chunk_data[6]|=4;
  }
  return true;
}


bool RDWaveFile::MakeLevl()
{
  for(int i=0;i<LEVL_CHUNK_SIZE;i++) {
    levl_chunk_data[i]=0;
  }
  WriteDword(levl_chunk_data,0,levl_version);
  WriteDword(levl_chunk_data,4,levl_format);       // Format
  WriteDword(levl_chunk_data,8,levl_points);       // Points per Value
  WriteDword(levl_chunk_data,12,levl_block_size);  // Blocksize
  WriteDword(levl_chunk_data,16,levl_channels);    // Channels
  WriteDword(levl_chunk_data,20,levl_frames);      // Total Peak Values
  WriteDword(levl_chunk_data,24,levl_peak_offset); // Offset to Peak-of-Peaks
  WriteDword(levl_chunk_data,28,132);              // Offset to Peak Data
  sprintf((char *)levl_chunk_data+32,"%s",
	  levl_timestamp.toString("yyyy:MM:dd:hh:mm:ss:000").toUtf8().
	  constData());

  return true;
}


QString RDWaveFile::cutString(char *buffer,unsigned start_point,unsigned size)
{
  QString string;

  for(unsigned i=start_point;i<start_point+size;i++) {
    if((0xFF&buffer[i])!=0) {
      string=string.append(buffer[i]);
    }
  }
  
  return string;
}
  



QDate RDWaveFile::cutDate(char *buffer,unsigned start_point)
{
  int i;
  char date_buf[11];
  int day=0,mon=0,year=0;

  for(i=0;i<4;i++) {
    date_buf[i]=buffer[start_point+i];
  }
  date_buf[4]=0;
  sscanf(date_buf,"%d",&year);
  if(year>RD_MAX_YEAR) {
    year=RD_MAX_YEAR;
  }
  for(i=0;i<2;i++) {
    date_buf[i]=buffer[start_point+i+5];
  }
  date_buf[2]=0;
  sscanf(date_buf,"%d",&mon);
  for(i=0;i<4;i++) {
    date_buf[i]=buffer[start_point+i+8];
  }
  date_buf[2]=0;
  sscanf(date_buf,"%d",&day);

  if((mon>0)&&(mon<13)&&(day>0)&&(day<32)) {
    return QDate(year,mon,day);
  }
  return QDate();
}




QTime RDWaveFile::cutTime(char *buffer,unsigned start_point)
{
  int i;
  char time_buf[9];
  int hour=0,min=0,sec=0;

  for(i=0;i<8;i++) {
    time_buf[i]=buffer[start_point+i];
  }
  time_buf[8]=0;
  sscanf(time_buf,"%d:%d:%d",&hour,&min,&sec);

  return QTime(hour,min,sec);
}


void RDWaveFile::WriteDword(unsigned char *buf,unsigned ptr,unsigned value)
{
  buf[ptr]=value&0xff;
  buf[ptr+1]=(value>>8)&0xff;
  buf[ptr+2]=(value>>16)&0xff;
  buf[ptr+3]=(value>>24)&0xff;
}


void RDWaveFile::WriteSword(unsigned char *buf,unsigned ptr,
			   unsigned short value)
{
  buf[ptr]=value&0xff;
  buf[ptr+1]=(value>>8)&0xff;
}


unsigned RDWaveFile::ReadDword(unsigned char *buffer,unsigned offset)
{
  return buffer[offset]+256*buffer[offset+1]+
    65536*buffer[offset+2]+16777216*buffer[offset+3];
}


unsigned short RDWaveFile::ReadSword(unsigned char *buffer,unsigned offset)
{
  return buffer[offset]+256*buffer[offset+1];
}


void RDWaveFile::GetEnergy()
{
  int file_ptr;

  ReadEnergyFile(wave_file_name);
  
  if(!levl_chunk) {
    GetLevl(wave_file.handle());
  }
  if(energy_loaded) {
    return;
  }
  file_ptr=lseek(wave_file.handle(),0,SEEK_CUR);
  lseek(wave_file.handle(),0,SEEK_SET);
  LoadEnergy();
  energy_loaded=true;
  lseek(wave_file.handle(),file_ptr,SEEK_SET);
}


unsigned RDWaveFile::LoadEnergy()
{
  unsigned i=0;
  unsigned char block[5];
  char pcm[4608];
  int block_size;
  int offset;
  unsigned energy_size;

  energy_data.clear();

  energy_size=getSampleLength()*getChannels()/1152;
  seekWave(0,SEEK_SET);
  switch(format_tag) {
  case WAVE_FORMAT_MPEG:
    if((head_layer==2)&&(mext_left_energy||mext_right_energy)) {
      while(i<energy_size) {
	lseek(wave_file.handle(),block_align-5,SEEK_CUR);
	if(read(wave_file.handle(),block,5)<5) {
	  has_energy=true;
	  return i;
	}
	if(mext_left_energy) {
	  energy_data.push_back(block[4]+256*block[3]);
	  i++;
	}
	if(mext_right_energy) {
	  energy_data.push_back(block[1]+256*block[0]);
	  i++;
	}
      }
      has_energy=true;
      return i;
    }
    else {
      has_energy=false;
      return 0;
    }
    break;

  case WAVE_FORMAT_PCM:
    switch(bits_per_sample) {
    case 16:
      block_size=2304*channels;
      while(i<energy_size) {
	if(read(wave_file.handle(),pcm,block_size)!=block_size) {
	  has_energy=true;
	  return i;
	}
	for(int j=0;j<channels;j++) {
	  energy_data.push_back(0);
	  for(int k=0;k<1152;k++) {
	    offset=2*k*channels+2*j;
	    if((pcm[offset]+256*pcm[offset+1])>energy_data[i]) {
	      energy_data[i]=pcm[offset]+256*pcm[offset+1];
	    }
	  }
	  i++;
	}
      }
      has_energy=true;
      return i;

    case 24:
      block_size=3456*channels;
      while(i<energy_size) {
	if(read(wave_file.handle(),pcm,block_size)!=block_size) {
	  has_energy=true;
	  return i;
	}
	for(int j=0;j<channels;j++) {
	  energy_data.push_back(0);
	  for(int k=0;k<1152;k++) {
	    offset=3*k*channels+3*j;
	    if((pcm[offset]+256*pcm[offset+1])>energy_data[i]) {
	      energy_data[i]=pcm[offset]+256*pcm[offset+1];
	    }
	  }
	  i++;
	}
      }
      has_energy=true;
      return i;
    }
    break;

  case WAVE_FORMAT_VORBIS:
    block_size=2304*channels;
    while(i<energy_size) {
      if(readWave(pcm,block_size)!=block_size) {
	has_energy=true;
	return i;
      }
      for(int j=0;j<channels;j++) {
	energy_data.push_back(0);
	for(int k=0;k<1152;k++) {
	  offset=2*k*channels+2*j;
	  if((pcm[offset]+256*pcm[offset+1])>energy_data[i]) {
	    energy_data[i]=pcm[offset]+256*pcm[offset+1];
	  }
	}
	i++;
      }
    }
    has_energy=true;
    return i;
    break;

  default:
    has_energy=false;
    return 0;
    break;
  }
  return 0;
}


bool RDWaveFile::ReadEnergyFile(QString wave_file_name)
{
  if(has_energy && energy_loaded) return true;
  /*
  QFile energy_file;
  QString str;
  unsigned char frame[50];

  energy_file.setName(wave_file_name+".energy");
  if(!energy_file.open(QIODevice::ReadOnly)) 
    return false;
  if(energy_file.readLine(str,20) <= 0)
     return false;
  normalize_level=str.toDouble();
  energy_file.close();
  if(!energy_file.open(QIODevice::ReadOnly)) 
    return false;
  read(energy_file.handle(),frame,str.length());
  int i=0;
  while(read(energy_file.handle(),frame,2)>0) {
      energy_data.push_back(frame[0]+256*frame[1]);
      i++;
  }
  ext_time_length=
   	(unsigned)(1000.0*(((double)i*1152)/getChannels())/(double)samples_per_sec);  
  energy_file.close();
  energy_loaded=true;
  has_energy=true;
  return true;
  */
  return false;
}


bool RDWaveFile::ReadNormalizeLevel(QString wave_file_name)
{
  QFile energy_file;
  char str[21];
  int n;

  //
  // FIXME: This is really evil!
  //
  energy_file.setFileName(wave_file_name+".energy");
  if(!energy_file.open(QIODevice::ReadOnly)) 
    return false;
  if((n=energy_file.readLine(str,20)) <= 0)
     return false;
  str[n]=0;
  normalize_level=QString(str).toDouble();
  energy_file.close();
  return true;
}


double RDWaveFile::getNormalizeLevel() const
{
  return normalize_level;
}


void RDWaveFile::setNormalizeLevel(double level)
{
  normalize_level=level;
}


void RDWaveFile::GrowAlloc(size_t size)
{
  if(size>(size_t)cook_buffer_size) {
    cook_buffer=(unsigned char *)realloc(cook_buffer,size);
    cook_buffer_size=size;
  }
}


#ifdef HAVE_VORBIS
int RDWaveFile::WriteOggPage(ogg_page *page)
{
  int n;

  n=write(wave_file.handle(),page->header,page->header_len);
  n+=write(wave_file.handle(),page->body,page->body_len);
  return n;
}
#endif  // HAVE_VORBIS


int RDWaveFile::WriteOggBuffer(char *buf,int size)
{
#ifdef HAVE_VORBIS
  float **buffer=vorbis_analysis_buffer(&vorbis_dsp,size/channels);
  for(int i=0;i<(size/(2*channels));i++) {
    for(int j=0;j<channels;j++) {
/*
      buffer[j][i]=
	(float)((buf[2*channels*i+2*j]&0xff)|
		((buf[2*channels*i+2*j+1]&0xff)<<8))/32768.0f;
*/
/*
      if(buffer[j][i]>high) {
	high=buffer[j][i];
      }
*/
      buffer[j][i]=
	((float)(buf[2*channels*i+2*j]&0xff)+
	 (256.0f*(float)(buf[2*channels*i+2*j+1]&0xff)))/32768.0f;
    }
//    printf("HIGH: %5.3f\n",high);
  }
  vorbis_analysis_wrote(&vorbis_dsp,size/(2*channels));
  while(vorbis_analysis_blockout(&vorbis_dsp,&vorbis_blk)==1) {
    vorbis_analysis(&vorbis_blk,NULL);
    vorbis_bitrate_addblock(&vorbis_blk);
    while(vorbis_bitrate_flushpacket(&vorbis_dsp,&ogg_pack)) {
      ogg_stream_packetin(&ogg_stream,&ogg_pack);
      while(ogg_stream_pageout(&ogg_stream,&ogg_pg)) {
	WriteOggPage(&ogg_pg);
      }
    }
  }
#endif  // HAVE_VORBIS
  return 0;
}


unsigned RDWaveFile::FrameOffset(int msecs) const
{
  if(msecs<0) {
    return 0;
  }
  return (unsigned)((double)msecs*(double)samples_per_sec/1000.0);
}


int RDWaveFile::CheckExitCode(const QString &msg,int exit_code)
{
  if(exit_code<0) {
    fprintf(stderr,"%s returned non-zero exit code %d [%s]\n",
	    msg.toUtf8().constData(),exit_code,strerror(errno));
  }
  return exit_code;
}
