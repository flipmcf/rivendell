// voicetracker.cpp
//
// Voice tracker panel for RDAirPlay
//
//   (C) Copyright 2021 Fred Gleason <fredg@paravelsystems.com>
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

#include "colors.h"
#include "voicetracker.h"

VoiceTracker::VoiceTracker(QWidget *parent)
  : RDWidget(parent)
{
  d_listlogs_dialog=new RDListLogs(RDLogFilter::StationFilter,"RDAirPlay",this);

  d_tracker_widget=new RDTrackerWidget(&d_import_path,this);
  if(rda->airplayConf()->barAction()==RDAirPlayConf::StartNext) {
    d_tracker_widget->setFocusPolicy(Qt::NoFocus);
  }
  else {
    d_tracker_widget->setFocusPolicy(Qt::StrongFocus);
  }

  d_load_button=new QPushButton(tr("Load\nLog"),this);
  d_load_button->setFont(bigButtonFont());
  d_load_button->setFocusPolicy(Qt::NoFocus);

  connect(d_load_button,SIGNAL(clicked()),this,SLOT(loadData()));
  connect(d_tracker_widget,SIGNAL(activeChanged(bool)),
	  d_load_button,SLOT(setDisabled(bool)));

  setFocusPolicy(Qt::NoFocus);
}


QSize VoiceTracker::sizeHint() const
{
  return QSize(10+d_tracker_widget->sizeHint().width(),
	       10+d_tracker_widget->sizeHint().height());
}


void VoiceTracker::loadData()
{
  QString logname;
  
  printf("HERE1\n");
  if(d_listlogs_dialog->exec(&logname)) {
    printf("HERE2: %s\n",logname.toUtf8().constData());
    if(d_tracker_widget->load(logname)) {
      d_load_button->disconnect();
      connect(d_load_button,SIGNAL(clicked()),this,SLOT(unloadData()));
      d_load_button->setText(tr("Unload\nLog"));
    }
  }
}


void VoiceTracker::unloadData()
{
  d_tracker_widget->unload();
  d_load_button->disconnect();
  connect(d_load_button,SIGNAL(clicked()),this,SLOT(loadData()));
  d_load_button->setText(tr("Load\nLog"));
}


void VoiceTracker::resizeEvent(QResizeEvent *e)
{
  d_tracker_widget->setGeometry(5,5,size().width()-10,size().height()-10);
  d_load_button->setGeometry(size().width()-95,size().height()-55,80,50);
}


void VoiceTracker::paintEvent(QPaintEvent *e)
{
  QPainter *p=new QPainter(this);
  p->fillRect(0,0,size().width(),size().height(),
	      QColor(AIR_WIDGET_BACKGROUND_COLOR));
}
