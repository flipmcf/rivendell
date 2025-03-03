// rdpadd.h
//
// Rivendell PAD Consolidation Server
//
//   (C) Copyright 2018-2024 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef RDPADD_H
#define RDPADD_H

#include <QList>
#include <QObject>

#include <rdconfig.h>

#include "repeater.h"

#define RDPADD_USAGE "\n\n"

class MainObject : public QObject
{
  Q_OBJECT
 public:
  MainObject();

 private:
  QList<Repeater *> d_repeaters;
  RDConfig *d_config;
};


#endif  // RDPADD_H
