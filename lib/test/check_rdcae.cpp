
/* compile with

g++ -c check_rdcae.cpp -o check_rdcae.o -fPIC -I/home/rd/repos/rivendell/lib -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets -I/usr/include/x86_64-linux-gnu/qt5/QtNetwork -I/usr/include/x86_64-linux-gnu/qt5/QtCore

g++ check_rdcae.o -o check_rdcae -lgtest -lgtest_main -lm -lpthread /home/rd/repos/rivendell/lib/.libs/librd.so -lcurl /usr/lib/x86_64-linux-gnu/libid3.so -ltag -lFLAC -lFLAC++ -lsndfile -lcdda_interface -lcdda_paranoia -lcrypt -ldl -lpam -lSoundTouch -lcrypto -lasound /home/rd/repos/rivendell/rdhpi/.libs/librdhpi.so -lhpi -ljack -lsamplerate -lvorbisfile -lvorbisenc -lQt5Sql -lQt5Xml -lQt5WebKitWidgets -lQt5Widgets -lQt5WebKit -lQt5Gui -lQt5Network -lQt5Core -ldiscid -lmusicbrainz5cc -lcoverartcc /usr/lib/x86_64-linux-gnu/libMagick++-6.Q16.so /usr/lib/x86_64-linux-gnu/libMagickWand-6.Q16.so /usr/lib/x86_64-linux-gnu/libMagickCore-6.Q16.so -L../lib -L../rdhpi -fopenmp -pthread
pkg-config --libs Qt5Core Qt5Widgets Qt5Network

maybe add ../.libs/rdcae.o to the list of object files during linking?

Run with:
check_rdcae
*/

#include <gtest/gtest.h>
#include <QObject>
#include "rdcae.h"


class MockStation : public RDStation {

};

TEST(RdcaeTest, connectHost){

    MockStation *dummy_station;
    RDConfig *dummy_config;
    QObject *dummy_parent = 0;

    QObject *test_object = new RDCae(dummy_station, dummy_config, dummy_parent);

    SUCCEED();
}

TEST(RdcaeTest, enableMetering){
    FAIL();
}

TEST(RdcaeTest, loadPlay){
    FAIL();
}

TEST(RdcaeTest, unloadPlay){
    FAIL();
}

TEST(RdcaeTest, positionPlay){
    FAIL();
}

TEST(RdcaeTest, play){
    FAIL();
}

TEST(RdcaeTest, stopPlay){
    FAIL();
}


TEST(RdcaeTest, loadRecord){
    FAIL();
}


TEST(RdcaeTest, unloadRecord){
    FAIL();
}

TEST(RdcaeTest, record){
    FAIL();
}

TEST(RdcaeTest, stopRecord){
    FAIL();
}

TEST(RdcaeTest, setClockSource){
    FAIL();
}

TEST(RdcaeTest, setInputVolume){
    FAIL();
}

TEST(RdcaeTest, setOutputVolume){
    FAIL();
}

TEST(RdcaeTest, fadeOutputVolume){
    FAIL();
}

TEST(RdcaeTest, setInputLevel){
    FAIL();
}

TEST(RdcaeTest, setOutputLevel){
    FAIL();
}

TEST(RdcaeTest, setInputMode){
    FAIL();
}

TEST(RdcaeTest, setOutputMode){
    FAIL();
}

TEST(RdcaeTest, setInputVOXLevel){
    FAIL();
}

TEST(RdcaeTest, setInputType){
    FAIL();
}

TEST(RdcaeTest, setPassthroughVolume){
    FAIL();
}

TEST(RdcaeTest, inputStatus){
    FAIL();
}

TEST(RdcaeTest, inputMeterUpdate){
    FAIL();
}

TEST(RdcaeTest, outputMeterUpdate){
    FAIL();
}

TEST(RdcaeTest, outputStreamMeterUpdate){
    FAIL();
}

TEST(RdcaeTest, playPosition){
    FAIL();
}

TEST(RdcaeTest, requestTimescale){
    FAIL();
}

TEST(RdcaeTest, playPortStatus){
    FAIL();
}

TEST(RdcaeTest, readyData){
    FAIL();
}

TEST(RdcaeTest, SendCommand){
    FAIL();
}

TEST(RdcaeTest, DispatchCommand){
    FAIL();
}


TEST(RdcaeTest, UpdateMeters){
    FAIL();
}

TEST(RdcaeTest, SerialCheck){
    FAIL();
}
