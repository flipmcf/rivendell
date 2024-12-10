## Makefile.am
##
## Automake.am for rivendell/lib/tests
##
## (C) Copyright 2024-2025 Fred Gleason <fredg@paravelsystems.com>, 
##                         Michael McFadden <flip@flipmcf.com>
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License version 2 as
##   published by the Free Software Foundation.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public
##   License along with this program; if not, write to the Free Software
##   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##
##
## Use automake to process this into a Makefile.in

## Notes for setting up googletest on Ubuntu 22.02

## 1. sudo apt install googletest
## 2. sudo apt install cmake 
## sudo mkdir /usr/local/lib/googletest   (this is your build directory)
## cd /usr/local/lib/googletest
## sudo cmake /usr/src/googletest  (sudo, so you can write to builddir)
## sudo make (same reason)
## sudo make install



## 2. READ /usr/share/doc/googletest
## 3. READ https://google.github.io/googletest/primer.html (syntax of test cases & test suites)

check_LTLIBRARIES = libgtest.la
libgtest_la_SOURCES /usr/src/googletest/googletest/src/gtest-all.cc
libgtest_la_CPPFLAGS = -I$(top_srcdir)/googletest/googletest/include/ -I$(top_srcdir)/googletest/googletest
libgtest_la_LDFLAGS = -pthread




