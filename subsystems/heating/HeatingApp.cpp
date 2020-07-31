#include "HeatingApp.hpp"

#include <components.hpp>

#include <thread>
#include <chrono>
#include <cstdlib>

HeatingApp::HeatingApp(QObject *parent, Components *components)
    : QObject (parent), mComponents(components)
{
}

void HeatingApp::run()
{   
    std::system("sudo owfs --i2c=ALL:ALL --allow_other /mnt/1wire/ --timeout_volatile=120");

    while(true)
    {
        mComponents->mHeatingComponents->mHeatingHardware.refreshTemperatureReading();
        for(auto terminal: mComponents->mControllers)
        {
            if(terminal.type == ControllerInfo::Type::TERMINALv1)
            {
                auto address = QHostAddress(terminal.ipAddr);
                mComponents->mHeatingComponents->updateTerminalCurrentTemperatures(address);
            }
        }
        std::this_thread::sleep_for(std::chrono::minutes(10));
    }
}

