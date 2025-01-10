
#include "gtest/gtest.h"
#include <QTest>
#include "rdcae.h"

namespace rd {
namespace lib {
namespace rdcae {
namespace {

class MockStation : public RDStation {

};

class MockRDConfig : public RDConfig {

};

class TestingThread : public QThread {
public:
    void run() override {
        QMutexLocker locker(&mutex);
        // Do some work in the thread
        condition.wakeOne();
    }

    QMutex mutex;
    QWaitCondition condition;
};

//Fixture
class RDCaeTest : public ::testing::Test {
  protected:
    RDCaeTest() {
        //init the QObject just enough to run it.
        // Thread here?  Probably not.
    }

    MockStation *dummy_station;
    RDConfig *dummy_config = new RDConfig();
    QObject *dummy_parent = 0;

  public:
    ~RDCaeTest override {
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

TEST_F(RdCaeTest, Construct){
    MockStation *dummy_station;
    RDConfig *dummy_config = new RDConfig();
    QObject *dummy_parent = 0;
    printf("calling constructor\n");
    
    QObject *test_object = new RDCae(dummy_station, dummy_config, dummy_parent);
    printf("constructed \n");
    delete test_object;
    SUCCEED();
}

TEST_F(RdCaeTest, connectHost){

    MockStation *dummy_station;
    RDConfig *dummy_config = new RDConfig();
    QObject *dummy_parent = 0;
    QString *error_msg = new QString();
    bool result;

    RDCae *test_object = new RDCae(dummy_station, dummy_config, dummy_parent);
    
    result = test_object->connectHost(error_msg);

    SUCCEED();
}

TEST_F(RdCaeTest, enableMetering){
    FAIL();
}

TEST_F(RdCaeTest, loadPlay){
    FAIL();
}
TEST_F(RdCaeTest, unloadPlay){
    FAIL();
}

TEST_F(RdCaeTest, positionPlay){
    FAIL();
}

TEST_F(RdCaeTest, play){
    FAIL();
}

TEST_F(RdCaeTest, stopPlay){
    FAIL();
}

TEST_F(RdCaeTest, loadRecord){
    FAIL();
}

TEST_F(RdCaeTest, unloadRecord){
    FAIL();
}

TEST_F(RdCaeTest, record){
    FAIL();
}

TEST_F(RdCaeTest, stopRecord){
    FAIL();
}

TEST_F(RdCaeTest, setClockSource){
    FAIL();
}

TEST_F(RdCaeTest, setInputVolume){
    FAIL();
}

TEST_F(RdCaeTest, setOutputVolume){
    FAIL();
}

TEST_F(RdCaeTest, fadeOutputVolume){
    FAIL();
}

TEST_F(RdCaeTest, setInputLevel){
    FAIL();
}

TEST_F(RdCaeTest, setOutputLevel){
    FAIL();
}

TEST_F(RdCaeTest, setInputMode){
    FAIL();
}

TEST_F(RdCaeTest, setOutputMode){
    FAIL();
}

TEST_F(RdCaeTest, setInputVOXLevel){
    FAIL();
}

TEST_F(RdCaeTest, setInputType){
    FAIL();
}

TEST_F(RdCaeTest, setPassthroughVolume){
    FAIL();
}

TEST_F(RdCaeTest, inputStatus){
    FAIL();
}

TEST_F(RdCaeTest, inputMeterUpdate){
    FAIL();
}

TEST_F(RdCaeTest, outputMeterUpdate){
    FAIL();
}

TEST_F(RdCaeTest, outputStreamMeterUpdate){
    FAIL();
}

TEST_F(RdCaeTest, playPosition){
    FAIL();
}

TEST_F(RdCaeTest, requestTimescale){
    FAIL();
}

TEST_F(RdCaeTest, playPortStatus){
    FAIL();
}

TEST_F(RdCaeTest, readyData){
    FAIL();
}

TEST_F(RdCaeTest, SendCommand){
    FAIL();
}

TEST_F(RdCaeTest, DispatchCommand){
    FAIL();
}

TEST_F(RdCaeTest, UpdateMeters){
    FAIL();
}

TEST_F(RdCaeTest, SerialCheck){
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

QTEST_MAIN(TestQString)
#include "testqstring.moc"