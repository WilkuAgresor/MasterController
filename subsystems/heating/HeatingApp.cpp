#include "HeatingApp.hpp"

#include <components.hpp>

#include <LeonardoIpExecutor/OpenthermMessages.hpp>

#include <thread>
#include <chrono>
#include <cstdlib>

HeatingApp::HeatingApp(QObject *parent, Components *components)
    : AppBase(parent, components)
{
}

void HeatingApp::run()
{   
    waitUntilInitialized();

    std::system("sudo owfs --i2c=ALL:ALL --allow_other /mnt/1wire/ --timeout_volatile=120");

    mComponents->mHeatingComponents->setupEmptyBoilerSettings();

    while(true)
    {
        QString boilerControllerId {"boilerController"};

        mComponents->mHeatingComponents->mHeatingHardware.refreshTemperatureReading();
        for(auto terminal: mComponents->mControllers)
        {
            if(terminal.type == ControllerInfo::Type::TERMINALv1)
            {
                auto address = QHostAddress(terminal.ipAddr);
                mComponents->mHeatingComponents->updateTerminalCurrentTemperatures(address);
                mComponents->mHeatingComponents->updateTerminalBoilerSettings(address);
            }
            else if(mComponents->mHeatingComponents->isBoilerControllerById(terminal.name))
            {
                qDebug() <<"periodic retrieve boiler settings";

                OpenthermGetMessage message;
                auto response = mComponents->mSender->sendReceiveRaw(terminal.getIpAddress(), terminal.port, message.serialize(), 1000);

                if(response.isEmpty())
                {
                    qDebug() << "Boiler controller unreachable";
                }
                else
                {
                    auto responseString = QString::fromUtf8(response);
                    auto settings = parseOpenthermGetResponse(responseString);
                    mComponents->mHeatingComponents->handleBoilerStateUpdate(settings);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(20));
    }
}

