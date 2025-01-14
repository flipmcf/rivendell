
#include "gtest/gtest.h"
#include "rdcae.h"

#include <QThread>
#include <QTimer>

// https://doc.qt.io/qt-5/qthread.html

namespace rd {
namespace lib {
namespace rdcae {
namespace {

class MockStation : public RDStation {

};

class MockRDConfig : public RDConfig {

};

//Fixture
class RDCaeTest : public ::testing::Test {

  public:
    ~RDCaeTest() override {
        //destory the QObject.
    }

  // Override this to define how to set up the environment.
  void SetUp() override {
    //QObject thread can be created here
  }

  // Override this to define how to tear down the environment.
  void TearDown() override {
    //QObject thread is cleanly destroyed here.
  }


};

TEST(RdCaeTest, Construct){
    MockStation *dummy_station;
    RDConfig *dummy_config = new RDConfig();
    QObject *dummy_parent = 0;
    
    QObject *test_object = new RDCae(dummy_station, dummy_config, dummy_parent);

    delete test_object;
    SUCCEED();
}

class ConnectHostThread : public QThread {
        
protected:
    void run() override {
        MockStation *dummy_station;
        RDConfig *dummy_config = new RDConfig();
        QObject *dummy_parent = 0;
        QString *error_msg = new QString();
    
        bool result;
        RDCae *test_object = new RDCae(dummy_station, dummy_config, dummy_parent);
        result = test_object->connectHost(error_msg);
    }
};

TEST(RdCaeTest, connectHost){
    ConnectHostThread thread;

    thread.start();

    // Wait for the thread to finish (you might need to use a QTimer or QEventLoop here)
    QTimer::singleShot(1000, [&]() {
        thread.quit();
        thread.wait();
    });

    ASSERT_TRUE(thread.isFinished());
}


TEST(RdCaeTest, enableMetering){
    FAIL();
}

TEST(RdCaeTest, loadPlay){
    FAIL();
}
TEST(RdCaeTest, unloadPlay){
    FAIL();
}

TEST(RdCaeTest, positionPlay){
    FAIL();
}

TEST(RdCaeTest, play){
    FAIL();
}

TEST(RdCaeTest, stopPlay){
    FAIL();
}

TEST(RdCaeTest, loadRecord){
    FAIL();
}

TEST(RdCaeTest, unloadRecord){
    FAIL();
}

TEST(RdCaeTest, record){
    FAIL();
}

TEST(RdCaeTest, stopRecord){
    FAIL();
}

TEST(RdCaeTest, setClockSource){
    FAIL();
}

TEST(RdCaeTest, setInputVolume){
    FAIL();
}

TEST(RdCaeTest, setOutputVolume){
    FAIL();
}

TEST(RdCaeTest, fadeOutputVolume){
    FAIL();
}

TEST(RdCaeTest, setInputLevel){
    FAIL();
}

TEST(RdCaeTest, setOutputLevel){
    FAIL();
}

TEST(RdCaeTest, setInputMode){
    FAIL();
}

TEST(RdCaeTest, setOutputMode){
    FAIL();
}

TEST(RdCaeTest, setInputVOXLevel){
    FAIL();
}

TEST(RdCaeTest, setInputType){
    FAIL();
}

TEST(RdCaeTest, setPassthroughVolume){
    FAIL();
}

TEST(RdCaeTest, inputStatus){
    FAIL();
}

TEST(RdCaeTest, inputMeterUpdate){
    FAIL();
}

TEST(RdCaeTest, outputMeterUpdate){
    FAIL();
}

TEST(RdCaeTest, outputStreamMeterUpdate){
    FAIL();
}

TEST(RdCaeTest, playPosition){
    FAIL();
}

TEST(RdCaeTest, requestTimescale){
    FAIL();
}

TEST(RdCaeTest, playPortStatus){
    FAIL();
}

TEST(RdCaeTest, readyData){
    FAIL();
}

TEST(RdCaeTest, SendCommand){
    FAIL();
}

TEST(RdCaeTest, DispatchCommand){
    FAIL();
}

TEST(RdCaeTest, UpdateMeters){
    FAIL();
}

TEST(RdCaeTest, SerialCheck){
    FAIL();
}

}  // namespace
}  // namespace rdcae
}  // namespace lib
}  // namespace rd


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
