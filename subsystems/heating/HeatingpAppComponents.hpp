#pragma once

#include <QNetworkDatagram>
#include <../common/subsystems/heating/HeatingZoneSettings.hpp>
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

    TemperatureSensorDataBank mSensorDataBank;
    Components* mSystemComponents;
};

