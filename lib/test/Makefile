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



# Instructions to make gtest library.
#
# ## Notes for setting up googletest on Ubuntu 22.02

## 1. Getubuntu packages
## 1. sudo apt install googletest
## 2. sudo apt install cmake 

## Setup:
##  create a simlink to the googletest src so future devs can use a custom src
## mkdir $(top_srcdir)/googletest
## ln -s /usr/src/googletest src
##  create a rivendell-specific build / binary directory for builds of googletest to link rivendell against
## mkdir $(top_srcdir)/googletest
##  


## 2. Build
## cd $(top_srcdir)/googletest
## cmake -DCMAKE_INSTALL_PREFIX:PATH=. src
## make
## sudo make install (mostly just updates pkgconfig)

## 2. READ /usr/share/doc/googletest
## 3. READ https://google.github.io/googletest/primer.html (syntax of test cases & test suites)
## READ AUTOMAKE example here https://ethz-adrl.github.io/ct/ct_core/doc/html/md__home_adrl_code_src_control-toolbox_ct_core_build_test_googletest-src_googletest_docs_Pkgconfig.html
## READ QT Unit testing: https://contribute.qt-project.org/wiki/Writing_Unit_Tests

## g++ -c check_rdcae.cpp -o check_rdcae.o -fPIC -I../lib -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets -I/usr/include/x86_64-linux-gnu/qt5/QtNetwork -I/usr/include/x86_64-linux-gnu/qt5/QtCore

## g++ check_rdcae.o -o check_rdcae -lgtest -lgtest_main -lm -lpthread /home/rd/repos/rivendell/lib/.libs/librd.so -lcurl /usr/lib/x86_64-linux-gnu/libid3.so -ltag -lFLAC -lFLAC++ -lsndfile -lcdda_interface -lcdda_paranoia -lcrypt -ldl -lpam -lSoundTouch -lcrypto -lasound /home/rd/repos/rivendell/rdhpi/.libs/librdhpi.so -lhpi -ljack -lsamplerate -lvorbisfile -lvorbisenc -lQt5Sql -lQt5Xml -lQt5WebKitWidgets -lQt5Widgets -lQt5WebKit -lQt5Gui -lQt5Network -lQt5Core -ldiscid -lmusicbrainz5cc -lcoverartcc /usr/lib/x86_64-linux-gnu/libMagick++-6.Q16.so /usr/lib/x86_64-linux-gnu/libMagickWand-6.Q16.so /usr/lib/x86_64-linux-gnu/libMagickCore-6.Q16.so -L../lib -L../rdhpi -fopenmp -pthread
## pkg-config --libs Qt5Core Qt5Widgets Qt5Network

CC=g++
CFLAGS=-I.
TESTS=check_rdlib

top_srcdir=/home/mcfaddenm/repos/rivendell

GTEST_LIB=$(top_srcdir)/googletest/lib
GTEST_INCLUDE=$(top_srcdir)/googletest/include
GTEST_SRC=$(top_srcdir)/googletest/src

## This is a great, simple test target example... The test file, and the file being tested

rdcae: $(top_srcdir)/lib/rdcae.cpp
		cd .. && make && cd test

test_rdcae: check_rdcae.cpp rdcae
		$(CC) -g check_rdcae.cpp -o check_rdcae \
		$(top_srcdir)/lib/rdcae.o $(top_srcdir)/lib/.libs/librd.so \
		-fPIC -I$(top_srcdir)/lib -I/usr/include/x86_64-linux-gnu/qt5 \
		-I/usr/include/x86_64-linux-gnu/qt5/QtWidgets -I/usr/include/x86_64-linux-gnu/qt5/QtNetwork \
		-I/usr/include/x86_64-linux-gnu/qt5/QtCore -I$(GTEST_INCLUDE) -lgtest -lgtest_main -lm \
		-lpthread  -lcurl \
		/usr/lib/x86_64-linux-gnu/libid3.so -ltag -lFLAC -lFLAC++ \
		-lsndfile -lcdda_interface -lcdda_paranoia -lcrypt -ldl -lpam \
		-lSoundTouch -lcrypto -lasound ../../rdhpi/.libs/librdhpi.so -lhpi -ljack \
		-lsamplerate -lvorbisfile -lvorbisenc -lQt5Sql -lQt5Xml -lQt5WebKitWidgets -lQt5Widgets -lQt5WebKit \
		-lQt5Gui -lQt5Network -lQt5Core -ldiscid -lmusicbrainz5cc -lcoverartcc /usr/lib/x86_64-linux-gnu/libMagick++-6.Q16.so \
		/usr/lib/x86_64-linux-gnu/libMagickWand-6.Q16.so /usr/lib/x86_64-linux-gnu/libMagickCore-6.Q16.so \
		-L$(top_srcdir)/rdhpi \
		-L$(GTEST_LIB) \
		-fopenmp -pthread


check: test_rdcae
	export LD_LIBRARY_PATH=$(top_srcdir)/lib/.lib
	./check_rdcae

# #this compiles the entire set of tests and suites and fixtures and everything.
# test: $(TESTS)

# # this RUNS the entire set of test suites (currently set to just run one)
# check: ./check_rdcae

# gtest_top = googletest/googletest
# gmock_top = googletest/googlemock

# check_LTLIBRARIES = libgtest.la libgmock.la

# libgtest_la_SOURCES = $(top_srcdir)/$(gtest_top)/src/gtest-all.cc
# libgtest_la_CPPFLAGS = \
#     -std=c++14 \
#     -isystem$(top_srcdir)/$(gtest_top)/include \
#     -isystem$(top_srcdir)/$(gtest_top)
# libgtest_la_LDFLAGS = -pthread

# libgmock_la_SOURCES = $(top_srcdir)/$(gmock_top)/src/gmock-all.cc
# libgmock_la_CPPFLAGS = \
#         -std=c++14 \
#         -isystem$(top_srcdir)/$(gtest_top)/include \
#         -isystem$(top_srcdir)/$(gmock_top)/include \
#         -isystem$(top_srcdir)/$(gmock_top)
# libgmock_la_LDFLAGS = -pthread

# EXTRA_DIST = \
#     options.txt

# check_PROGRAMS = check_rdlib
# noinst_PROGRAMS = check_rdlib

# check_rdlib_SOURCES = check_rdlib.cpp

# tests_check_rdlib_LDADD = libgtest.la libgmock.la
# tests_check_rdlib_LDFLAGS = -lstdc++ -lm -shared-libgcc $(LDFLAGS)
# tests_check_rdlib_CFLAGS =  $(CFLAGS) $(CHECK_CFLAGS) $(gtest_CPPFLAGS)





# CLEANFILES = *~\
#              moc_*\
#              *.lib\
#              *.obj\
#              *.qm