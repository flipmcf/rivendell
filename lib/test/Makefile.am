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

##gcc -c check_rdlib.o
##gcc check_rdlib.o -o check_rdlib -xc++ -lgtest -lgtest_main -lstdc++ -shared-libgcc -lm


# Instructions to make gtest library.
#
gtest_top = googletest/googletest
gmock_top = googletest/googlemock

check_LTLIBRARIES = libgtest.la libgmock.la

libgtest_la_SOURCES = $(top_srcdir)/$(gtest_top)/src/gtest-all.cc
libgtest_la_CPPFLAGS = \
    -std=c++14 \
    -isystem$(top_srcdir)/$(gtest_top)/include \
    -isystem$(top_srcdir)/$(gtest_top)
libgtest_la_LDFLAGS = -pthread

libgmock_la_SOURCES = $(top_srcdir)/$(gmock_top)/src/gmock-all.cc
libgmock_la_CPPFLAGS = \
        -std=c++14 \
        -isystem$(top_srcdir)/$(gtest_top)/include \
        -isystem$(top_srcdir)/$(gmock_top)/include \
        -isystem$(top_srcdir)/$(gmock_top)
libgmock_la_LDFLAGS = -pthread

EXTRA_DIST = \
    options.txt

check_PROGRAMS = check_rdlib
noinst_PROGRAMS = check_rdlib

check_rdlib_SOURCES = check_rdlib.cpp

tests_check_rdlib_LDADD = libgtest.la libgmock.la
tests_check_rdlib_LDFLAGS = -lstdc++ -lm -shared-libgcc $(LDFLAGS)
tests_check_rdlib_CFLAGS =  $(CFLAGS) $(CHECK_CFLAGS) $(gtest_CPPFLAGS)



TESTS = check_rdlib


CLEANFILES = *~\
             moc_*\
             *.lib\
             *.obj\
             *.qm