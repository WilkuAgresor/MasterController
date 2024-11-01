#pragma once

#include <QNetworkDatagram>
#include <../common/subsystems/heating/HeatingZoneSettings.hpp>
#include <../common/subsystems/heating/HeatingRetrieveMessage.hpp>
#include <../common/CommonDefinitions.hpp>
#include <../common/subsystems/heating/HeatingDictionary.hpp>
#include <../common/subsystems/heating/HeatingRetrieveStatistics.hpp>
#include <../common/subsystems/heating/BoilerSettingsMessage.hpp>
#include <subsystems/heating/SensorData.hpp>

#include <../common/subsystems/status/ControllerInfo.hpp>

class Components;

class HeatingAppComponents: public QObject
{
    Q_OBJECT
public:
    HeatingAppComponents(QObject* parent, Components* components);

    HeatSettingsPayload getCurrentHeatStatus();
    void reprovisionTerminalData(QHostAddress terminalAddr);
    void updateTerminalCurrentTemperatures(QHostAddress terminalAddr);
    void updateTerminalBoilerSettings(QHostAddress terminalAddr);

    void handleMessage(const Message& message, QHostAddress fromAddr);
    void handleSettingsUpdate(const HeatSettingsMessage &message, QHostAddress fromAddr);
    void handleSettingsRetrieve(const HeatRetrieveMessage &message, QHostAddress fromAddr);
    void handleStatisticsRetrieve(const HeatRetrieveStatisticsMessage &message, QHostAddress fromAddr);
    void handleBoilerSettingsUpdate(const BoilerSettingsMessage &message, QHostAddress fromAddr);   

    //update boiler hardware
    void updateBoilerHardware(const QString& name, int value);

    //update database after boiler status retrieve from boilerController
    void handleBoilerStateUpdate(const BoilerSettingsPayload &boilerSettings);

    bool isBoilerControllerById(const QString& controllerId);

    void setupEmptyBoilerSettings();

    Components* mSystemComponents;
    HeatingHardware mHeatingHardware;
public slots:
    void handleHardwareReprovisionNotif(ControllerInfo controller);

private:
    std::mutex mDbMutex;
};

