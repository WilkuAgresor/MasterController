#include "TopologyApp.hpp"
#include <../common/CommonDefinitions.hpp>
#include <../common/subsystems/topology/topologyMessages.hpp>
#include <thread>
#include <chrono>

TopologyApp::TopologyApp(QObject *parent, Components &components)
    : QObject (parent), mComponents(components)
{
}

void TopologyApp::run()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));

        for(auto& controller: mComponents.mControllers)
        {
            if( controller.type == ControllerInfo::Type::TERMINALv1)
            {
                qDebug() << "TopologyApp: sending checkin request " << controller.ipAddr;
                TopologyRequestCheckinMessage message;
                QHostAddress address(controller.ipAddr);
                mComponents.mSender->send(address,TERMINAL_LISTEN_PORT, message.toData());
            }
        }
    }
}
