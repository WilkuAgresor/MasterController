#include "TopologyApp.hpp"
#include <../common/CommonDefinitions.hpp>
#include <../common/subsystems/topology/topologyMessages.hpp>
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

    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));

        for(auto& controller: mComponents->mControllers)
        {
            if( controller.type == ControllerInfo::Type::TERMINALv1)
            {
                qDebug() << "TopologyApp: sending checkin request " << controller.ipAddr;
                TopologyRequestCheckinMessage message;
                auto header = message.getHeader();
                header.mReplyPort = SERVER_LISTEN_PORT;
                message.updateHeader(header);

                QHostAddress address(controller.ipAddr);
                mComponents->mSender->send(address,TERMINAL_LISTEN_PORT, message.toData());
            }
        }
    }
}
