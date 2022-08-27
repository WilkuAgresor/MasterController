#include "mainapplication.hpp"
#include <limits>
#include <random>
#include <../common/subsystems/alarm/alarmSetMessage.hpp>

#include <chrono>
#include <thread>
#include <QDebug>

MainApplication::MainApplication(QObject *parent) : QObject(parent)
  , mKeyKeeper(this)
  , mComponents(new Components(parent))
  , mHeatingApp(new HeatingApp(parent, mComponents.get()))
  , mTopologyApp(new TopologyApp(parent, mComponents.get()))
  , mTerminalListener(new TerminalListener(mComponents.get(), this, SERVER_LISTEN_PORT))
  , mLightsApp(new LightsApp(this, mComponents.get()))
{
    QThreadPool::globalInstance()->start(mHeatingApp);
    QThreadPool::globalInstance()->start(mTopologyApp);
    QThreadPool::globalInstance()->start(mLightsApp);
}

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


