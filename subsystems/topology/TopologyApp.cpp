#include "TopologyApp.hpp"
#include <../common/CommonDefinitions.hpp>
#include <../common/subsystems/topology/topologyMessages.hpp>
#include <LeonardoIpExecutor/LeoMessage.hpp>

#include <thread>
#include <chrono>

TopologyApp::TopologyApp(QObject *parent, Components *components)
    : AppBase(parent, components)
{
}

TopologyApp::~TopologyApp()
{
    for(auto& controller: mComponents->mControllers)
    {
        MasterShutdownMessage message;

        if( controller.type == ControllerInfo::Type::TERMINALv1)
        {
            QHostAddress address (controller.ipAddr);
            mComponents->mSender->send(address,TERMINAL_LISTEN_PORT, message.toData());
        }
    }
}

void TopologyApp::run()
{
    std::this_thread::sleep_for(std::chrono::seconds(10));

    while(true)
    {
        for(auto& controller: mComponents->mControllers)
        {
            if(controller.type == ControllerInfo::Type::TERMINALv1)
            {
                qDebug() << "TopologyApp: sending checkin request " << controller.ipAddr;
                TopologyRequestCheckinMessage message;
                auto header = message.getHeader();
                header.mReplyPort = SERVER_LISTEN_PORT;
                message.updateHeader(header);

                QHostAddress address(controller.ipAddr);
                mComponents->mSender->send(address,TERMINAL_LISTEN_PORT, message.toData());
            }
            if(controller.type == ControllerInfo::Type::ARD_LEO)
            {
                qDebug() << "TopologyApp: sending checkin request for ArdLeo "<<controller.ipAddr;
                LeoGetSessionIdMessage message;
                auto response = mComponents->mSender->sendReceiveRaw(controller.getIpAddress(), controller.port, message.serialize(), 300);

                if(response.isEmpty())
                {
                    qDebug() << "controller unreachable";
                }
                else
                {
                    auto responseMsgStr = QString::fromUtf8(response);
                    qDebug() << "reply message: "<< responseMsgStr;
                    LeoSessionIdReplyMessage responseMsg(responseMsgStr);

                    auto forceControllerUpdate = QFile::exists(sForceControllerConfigurationFlagFile);

                    if(forceControllerUpdate || (responseMsg.getSessionId() != 0 && responseMsg.getSessionId() != controller.key))
                    {                        
                        {
                            auto database = DatabaseFactory::createDatabaseConnection("topology");
                            database->updateControllerCurKey(controller.name, responseMsg.getSessionId());
                        }
                        qDebug() <<"NEW session ID: "<<responseMsg.getSessionId();
                        controller.key = responseMsg.getSessionId();
                        mComponents->sendHardwareReprovisionNotif(controller);                        
                    }
                    else if(responseMsg.getSessionId() == 0)
                    {
                        qDebug() << "Controller OFFLINE: "<<controller.ipAddr;
                    }
                    else
                    {
                        qDebug() << "Known session ID";
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(25));
    }
}
