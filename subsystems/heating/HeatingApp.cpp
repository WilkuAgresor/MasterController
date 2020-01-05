#include "HeatingApp.hpp"

#include <components.hpp>

#include <thread>
#include <chrono>

HeatingApp::HeatingApp(QObject *parent, Components *components)
    : QObject (parent), mComponents(components)
{
}

void HeatingApp::run()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        auto payload = mComponents.getCurrentHeatStatus();
        HeatSettingsMessage message(payload);
        QHostAddress address(QHostAddress::LocalHost);

        mComponents.mSystemComponents->mSender->send(address,TERMINAL_LISTEN_PORT, message.toData());
    }
}

