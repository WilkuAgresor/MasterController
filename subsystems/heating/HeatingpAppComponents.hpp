#pragma once

#include <QNetworkDatagram>
#include <../common/subsystems/heating/HeatingZoneSettings.hpp>
#include <../common/subsystems/heating/HeatingRetrieveMessage.hpp>
#include <../common/CommonDefinitions.hpp>
#include <../common/subsystems/heating/HeatingDictionary.hpp>
#include <subsystems/heating/SensorData.hpp>


class Components;

class HeatingAppComponents
{
public:
    HeatingAppComponents(Components* components);

    HeatSettingsPayload getCurrentHeatStatus();
    void reprovisionTerminalData(QHostAddress terminalAddr);
    void handleMessage(const Message& message, QHostAddress fromAddr, int fromPort);
    void handleSettingsUpdate(const HeatSettingsMessage &message, QHostAddress fromAddr, int fromPort);
    void handleSettingsRetrieve(const HeatRetrieveMessage &message, QHostAddress fromAddr);


    TemperatureSensorDataBank mSensorDataBank;
    Components* mSystemComponents;
};

