// rdwidget.h
//
// Base class for Rivendell widgets.
//
//   (C) Copyright 2019-2023 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef RDWIDGET_H
#define RDWIDGET_H

#include <QWidget>

#include <rdfontengine.h>

class RDWidget : public QWidget, public RDFontEngine
{
  Q_OBJECT;
 public:
  RDWidget(QWidget *parent=0,Qt::WindowFlags f=Qt::WindowFlags());
  RDWidget(RDConfig *config,QWidget *parent=0,Qt::WindowFlags f=Qt::WindowFlags());
  QPixmap backgroundPixmap() const;
  void setBackgroundPixmap(const QPixmap &pix);

 protected:
  void paintEvent(QPaintEvent *e);

 private:
  QPixmap d_background_pixmap;
};


#endif  // RDWIDGET_H
