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

# Instructions to make gtest library.
#

## 1. Get googletest ubuntu package and cmake
## 1. sudo apt install googletest
## 2. sudo apt install cmake 

## 2. Build
## sudo mkdir /usr/local/lib/googletest   (this is your build directory)
## cd /usr/local/lib/googletest
## sudo cmake /usr/src/googletest  (sudo, so you can write to builddir)
## sudo make (same reason)
## sudo make install (mostly just updates pkgconfig)

## 2. READ /usr/share/doc/googletest
## 3. READ https://google.github.io/googletest/primer.html (syntax of test cases & test suites)
## READ AUTOMAKE example here https://ethz-adrl.github.io/ct/ct_core/doc/html/md__home_adrl_code_src_control-toolbox_ct_core_build_test_googletest-src_googletest_docs_Pkgconfig.html

## g++ -c check_rdcae.cpp -o check_rdcae.o -fPIC -I/home/rd/repos/rivendell/lib -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets -I/usr/include/x86_64-linux-gnu/qt5/QtNetwork -I/usr/include/x86_64-linux-gnu/qt5/QtCore

## g++ check_rdcae.o -o check_rdcae -lgtest -lgtest_main -lm -lpthread /home/rd/repos/rivendell/lib/.libs/librd.so -lcurl /usr/lib/x86_64-linux-gnu/libid3.so -ltag -lFLAC -lFLAC++ -lsndfile -lcdda_interface -lcdda_paranoia -lcrypt -ldl -lpam -lSoundTouch -lcrypto -lasound /home/rd/repos/rivendell/rdhpi/.libs/librdhpi.so -lhpi -ljack -lsamplerate -lvorbisfile -lvorbisenc -lQt5Sql -lQt5Xml -lQt5WebKitWidgets -lQt5Widgets -lQt5WebKit -lQt5Gui -lQt5Network -lQt5Core -ldiscid -lmusicbrainz5cc -lcoverartcc /usr/lib/x86_64-linux-gnu/libMagick++-6.Q16.so /usr/lib/x86_64-linux-gnu/libMagickWand-6.Q16.so /usr/lib/x86_64-linux-gnu/libMagickCore-6.Q16.so -L../lib -L../rdhpi -fopenmp -pthread
## pkg-config --libs Qt5Core Qt5Widgets Qt5Network


CC=g++
CFLAGS=-I.
TESTS=check_rdlib

## This is a great, simple test target example... The test file, and the file being teste

rdcae: ../rdcae.cpp
		cd .. && make && cd test

test_rdcae: check_rdcae.cpp rdcae
		$(CC) check_rdcae.cpp -o check_rdcae \
		-fPIC -I/home/rd/repos/rivendell/lib -I/usr/include/x86_64-linux-gnu/qt5 \
		-I/usr/include/x86_64-linux-gnu/qt5/QtWidgets -I/usr/include/x86_64-linux-gnu/qt5/QtNetwork \
		-I/usr/include/x86_64-linux-gnu/qt5/QtCore -lgtest -lgtest_main -lm \
		-lpthread /home/rd/repos/rivendell/lib/.libs/librd.so -lcurl \
		/usr/lib/x86_64-linux-gnu/libid3.so -ltag -lFLAC -lFLAC++ \
		-lsndfile -lcdda_interface -lcdda_paranoia -lcrypt -ldl -lpam \
		-lSoundTouch -lcrypto -lasound /home/rd/repos/rivendell/rdhpi/.libs/librdhpi.so -lhpi -ljack \
		-lsamplerate -lvorbisfile -lvorbisenc -lQt5Sql -lQt5Xml -lQt5WebKitWidgets -lQt5Widgets -lQt5WebKit \
		-lQt5Gui -lQt5Network -lQt5Core -ldiscid -lmusicbrainz5cc -lcoverartcc /usr/lib/x86_64-linux-gnu/libMagick++-6.Q16.so \
		/usr/lib/x86_64-linux-gnu/libMagickWand-6.Q16.so /usr/lib/x86_64-linux-gnu/libMagickCore-6.Q16.so \
		-L../lib/.libs/rdcae.o -L../lib/.libs -L../rdhpi -fopenmp -pthread


check: test_rdcae
	EXPORT LD_LIBRARY_PATH=/home/rd/repos/rivendell/lib/.lib
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