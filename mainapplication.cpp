#include "mainapplication.hpp"
#include <limits>
#include <random>
#include <../common/subsystems/alarm/alarmSetMessage.hpp>

#include <chrono>
#include <thread>

MainApplication::MainApplication(QObject *parent) : QObject(parent)
  , mKeyKeeper(this)
  , mComponents(new Components(parent))
  , mHeatingApp(new HeatingApp(parent, mComponents.get()))
  , mTopologyApp(new TopologyApp(parent, *mComponents))
  , mTerminalListener(new TerminalListener(mComponents->mSender, mComponents.get(), this, SERVER_LISTEN_PORT))
{
    QThreadPool::globalInstance()->start(mHeatingApp);
    QThreadPool::globalInstance()->start(mTopologyApp);
}
//    //TESTS:
//    qDebug() << mDb.getDevicesControllerInfo("Elektrozaczep").print();

//    qDebug() <<"device json:"<< mDb.getDeviceInfo("Elektrozaczep").toString();

//    auto devInfo = mDb.getDeviceInfo("Elektrozaczep");

//    AlarmSetPayload payload(AlarmStateType::ARMED, DetectorPatternType::LIST, {"detector1", "detector2", "detector3"});
//    AlarmSetMessage message(payload);
//    qDebug() << "first message: "<< message.toString();

//    Message msg(message.toData());

//    auto header = msg.getHeader();
//    if(header.getType() == MessageType::ALARM_SET)
//    {
//        qDebug() <<"alarm set message";
//        auto& alarmMsg = static_cast<AlarmSetMessage&>(msg);
//        qDebug() <<"after cast: "<< alarmMsg.toString();
//        auto pd = alarmMsg.payload();
//        qDebug() << "pd: "<<pd.toString();
//    }



uint64_t MainApplication::generateNewKey()
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<uint64_t> dist(0,std::numeric_limits<uint64_t>::max());

    return dist(mt);
}

void MainApplication::changeConnectionState(QString controllerName, ConnectionState state)
{
    if(state == ConnectionState::FAILURE)
        emit finished(controllerName, "Failed");
    else if(state == ConnectionState::TIME_OUT)
    {
        mKeyKeeper.eraseEntry(controllerName);
        emit finished(controllerName, "Timed out");
    }
    else if(state == ConnectionState::SUCCESS)
        emit finished(controllerName, "Success");
    else if(state == ConnectionState::IN_PROGRESS)
        emit inProgress(controllerName);
}


