#include "messageHandler.hpp"
#include <../common/subsystems/alarm/alarmSetMessage.hpp>
#include <../common/messages/replyMessage.hpp>


#include <chrono>
#include <thread>

std::mutex MessageHandler::mMutex;

MessageHandler::MessageHandler(QNetworkDatagram datagram, Components *components)
    : mDatagram(datagram), mComponents(components)
{

}

void MessageHandler::run()
{
    std::lock_guard<std::mutex> _lock(mMutex);

    Message msg(mDatagram.data());
    auto database = DatabaseFactory::createDatabaseConnection("messageHandler");

    qDebug() << "Incoming message: "<< msg.toString();

    auto header = msg.getHeader();

    if(header.getType() == MessageType::ALARM_SET)
    {
        auto& alarmMsg = static_cast<AlarmSetMessage&>(msg);
        auto payload = alarmMsg.payload();

        qDebug() << "got alarm message: " <<payload.toString();

//        using namespace std::chrono;
//        for(int i = 0; i<5; i++)
//        {
//            std::this_thread::sleep_for(milliseconds(1000));
//            qDebug() << "in sleep "<< i;
//        }

        ReplyPayload repPayload(Status::OK);
        ReplyMessage repMsg(repPayload);

        emit result(mDatagram.makeReply(repMsg.toData()));
    }
    else if(header.getType() == MessageType::TOPOLOGY_CHECKIN)
    {
        auto addr = mDatagram.senderAddress();
        for(auto& controller: mComponents->mControllers)
        {
            if(controller.ipAddr == addr.toString())
            {
                qDebug() << "topology checkin from " <<addr.toString();
                controller.status = ControllerInfo::Status::ACTIVE;
            }
        }
    }
    else if(header.getType() == MessageType::TOPOLOGY_REQUEST_INIT)
    {
        qDebug() << "topology request init";
        mComponents->mHeatingComponents->reprovisionTerminalData(mDatagram.senderAddress());
    }
}
