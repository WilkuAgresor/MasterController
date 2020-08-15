#pragma once

#include <QObject>
#include <database/database.hpp>
#include <../common/sender.hpp>
#include <subsystems/heating/HeatingpAppComponents.hpp>
#include <subsystems/lights/LightsAppComponents.hpp>
#include <hardware/GrandCentral.hpp>

class Components : public QObject
{
public:
    Components(QObject* parent);
    ~Components() = default;

    ReceivePortsBank mNetworkPortRepository;
    Sender* mSender;
    std::vector<ControllerInfo> mControllers;
    std::unique_ptr<HeatingAppComponents> mHeatingComponents;
    LightsAppComponents* mLightsComponents;

    GrandCentral* mGrandCentral;
//    SerialConnection* mSerialConnection;


};
