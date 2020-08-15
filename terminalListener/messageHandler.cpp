#include "messageHandler.hpp"
#include <../common/subsystems/alarm/alarmSetMessage.hpp>
#include <../common/messages/replyMessage.hpp>


#include <chrono>
#include <thread>

std::mutex MessageHandler::mMutex;

MessageHandler::MessageHandler(QObject *parent, Message &&msg, QHostAddress fromAddr, Components *components)
    : QObject(parent), mMsg(msg), mFromAddr(fromAddr), mComponents(components)
{

}

void MessageHandler::run()
{
    std::lock_guard<std::mutex> _lock(mMutex);

    if(!mMsg.isValid())
    {
        qWarning() << "invalid message received";
    }

    qDebug() << "Incoming message: "<< mMsg.toString();

    auto header = mMsg.getHeader();

    if(header.getType() == MessageType::ALARM_SET)
    {
        auto& alarmMsg = static_cast<AlarmSetMessage&>(mMsg);
        auto payload = alarmMsg.payload();

        qDebug() << "got alarm message: " <<payload.toString();

    }
    else if(header.getType() == MessageType::TOPOLOGY_CHECKIN)
    {
        for(auto& controller: mComponents->mControllers)
        {
            if(controller.ipAddr == mFromAddr.toString())
            {
                qDebug() << "topology checkin from " <<mFromAddr.toString();
                controller.status = ControllerInfo::Status::ACTIVE;
            }
        }
    }
    else if(header.getType() == MessageType::TOPOLOGY_REQUEST_INIT)
    {
        qDebug() << "topology request init";
        mComponents->mHeatingComponents->reprovisionTerminalData(mFromAddr);
        mComponents->mLightsComponents->reprovisionTerminalData(mFromAddr);
    }
    else if(isHeatingMessage(header.getType()))
    {
        mComponents->mHeatingComponents->handleMessage(mMsg, mFromAddr);
    }
    else if(isLightsMessage(header.getType()))
    {
        mComponents->mLightsComponents->handleMessage(mMsg, mFromAddr);
    }
}
