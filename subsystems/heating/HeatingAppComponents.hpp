#pragma once

#include <QNetworkDatagram>
#include <../common/subsystems/heating/HeatingZoneSettings.hpp>
#include <../common/subsystems/heating/HeatingRetrieveMessage.hpp>
#include <../common/CommonDefinitions.hpp>
#include <../common/subsystems/heating/HeatingDictionary.hpp>
#include <../common/subsystems/heating/HeatingRetrieveStatistics.hpp>
#include <subsystems/heating/SensorData.hpp>


class Components;

class HeatingAppComponents
{
public:
    HeatingAppComponents(Components* components);

    HeatSettingsPayload getCurrentHeatStatus();
    void reprovisionTerminalData(QHostAddress terminalAddr);
    void updateTerminalCurrentTemperatures(QHostAddress terminalAddr);

    void handleMessage(const Message& message, QHostAddress fromAddr);
    void handleSettingsUpdate(const HeatSettingsMessage &message, QHostAddress fromAddr);
    void handleSettingsRetrieve(const HeatRetrieveMessage &message, QHostAddress fromAddr);
    void handleStatisticsRetrieve(const HeatRetrieveStatisticsMessage &message, QHostAddress fromAddr);


    HeatingHardware mHeatingHardware;
    Components* mSystemComponents;
    std::mutex mDbMutex;
};

