// add_feed.cpp
//
// Add a Rivendell Service
//
//   (C) Copyright 2002,2016 Fred Gleason <fredg@paravelsystems.com>
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

#include <qdialog.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qdatetime.h>

#include <rddb.h>
#include <rdtextvalidator.h>
#include <rdescape_string.h>
#include <rdcreateauxfieldstable.h>
#include <rdfeedlog.h>

#include <edit_feed.h>
#include <add_feed.h>

AddFeed::AddFeed(unsigned *id,QString *keyname,QWidget *parent)
  : QDialog(parent,"",true)
{
  feed_keyname=keyname;
  feed_id=id;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  setCaption(tr("Add Feed"));

  //
  // Create Fonts
  //
  QFont font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);
  QFont user_font=QFont("Helvetica",12,QFont::Normal);
  user_font.setPixelSize(12);

  //
  // Enable Users Checkbox
  //
  feed_users_box=new QCheckBox(this);
  feed_users_box->setGeometry(40,40,15,15);
  feed_users_box->setChecked(true);
  QLabel *label=new QLabel(feed_users_box,tr("Enable Feed for All Users"),this);
  label->setGeometry(60,38,sizeHint().width()-60,19);
  label->setFont(user_font);
  label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

  //
  // Text Validator
  //
  RDTextValidator *validator=new RDTextValidator(this,"validator");

  //
  // Feed Name
  //
  feed_keyname_edit=new QLineEdit(this);
  feed_keyname_edit->setGeometry(145,11,sizeHint().width()-150,19);
  feed_keyname_edit->setMaxLength(8);
  feed_keyname_edit->setValidator(validator);
  label=new QLabel(feed_keyname_edit,tr("&New Feed Name:"),this);
  label->setGeometry(10,11,130,19);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  //  Ok Button
  //
  QPushButton *ok_button=new QPushButton(this);
  ok_button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,80,50);
  ok_button->setDefault(true);
  ok_button->setFont(font);
  ok_button->setText(tr("&OK"));
  connect(ok_button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  //  Cancel Button
  //
  QPushButton *cancel_button=new QPushButton(this);
  cancel_button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,
			     80,50);
  cancel_button->setFont(font);
  cancel_button->setText(tr("&Cancel"));
  connect(cancel_button,SIGNAL(clicked()),this,SLOT(cancelData()));
}


AddFeed::~AddFeed()
{
  delete feed_keyname_edit;
}


QSize AddFeed::sizeHint() const
{
  return QSize(250,124);
} 


QSizePolicy AddFeed::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void AddFeed::okData()
{
  QString sql;
  RDSqlQuery *q;
  RDSqlQuery *q1;

  sql=QString("select KEY_NAME from FEEDS where ")+
    "KEY_NAME=\""+RDEscapeString(feed_keyname_edit->text())+"\"";
  q=new RDSqlQuery(sql);
  if(q->first()) {
    QMessageBox::warning(this,tr("Add Feed Error"),
			 tr("A feed with that key name already exists!"));
    delete q;
    return;
  }
  delete q;

  //
  // Create Default Feed Perms
  //
  if(feed_users_box->isChecked()) {
    sql="select LOGIN_NAME from USERS \
         where (ADMIN_USERS_PRIV='N')&&(ADMIN_CONFIG_PRIV='N')";
    q=new RDSqlQuery(sql);
    while(q->next()) {
      sql=QString("insert into FEED_PERMS set ")+
	"USER_NAME=\""+RDEscapeString(q->value(0).toString())+"\","+
	"KEY_NAME=\""+RDEscapeString(feed_keyname_edit->text())+"\"";
      q1=new RDSqlQuery(sql);
      delete q1;
    }
    delete q;
  }

  //
  // Create Feed
  //
  sql=QString("insert into FEEDS set ")+
    "KEY_NAME=\""+RDEscapeString(feed_keyname_edit->text())+"\","+
    "ORIGIN_DATETIME=\""+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+"\","+
    "HEADER_XML=\""+RDEscapeString(DEFAULT_HEADER_XML)+"\","+
    "CHANNEL_XML=\""+RDEscapeString(DEFAULT_CHANNEL_XML)+"\","+
    "ITEM_XML=\""+RDEscapeString(DEFAULT_ITEM_XML)+"\"";
  q=new RDSqlQuery(sql);
  delete q;
  RDCreateFeedLog(feed_keyname_edit->text());
  RDCreateAuxFieldsTable(feed_keyname_edit->text());
  sql=QString("select LAST_INSERT_ID() from FEEDS");
  q=new RDSqlQuery(sql);
  if(q->first()) {
    *feed_id=q->value(0).toUInt();
  }
  delete q;
  *feed_keyname=feed_keyname_edit->text();
  done(0);
}


void AddFeed::cancelData()
{
  done(-1);
}
