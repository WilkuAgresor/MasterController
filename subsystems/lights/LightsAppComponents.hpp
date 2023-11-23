#pragma once

#include <QNetworkDatagram>
#include <../common/subsystems/lights/LightControllerSettings.hpp>
#include <../common/subsystems/lights/LightsRetrieveMessage.hpp>
#include <../common/CommonDefinitions.hpp>
#include <../common/subsystems/lights/LightsDictionary.hpp>
#include <hardware/ControllerInfo.hpp>
#include <hardware/PinIdentifier.hpp>
#include <mutex>

class Components;

class LightsAppComponents : public QObject
{
    Q_OBJECT
public:
    LightsAppComponents(QObject* parent, Components* components);

    void reprovisionTerminalData(QHostAddress terminalAddr);
    void reprovisionHardwareState();
    void sendToTerminals(const LightSettingsMessage &message, QHostAddress fromAddr);

    void handleMessage(const Message& message, QHostAddress fromAddr);

    void handleClicked(int lightId);

    void handleLightsUpdateFromTerminal(const LightSettingsMessage &message, QHostAddress fromAddr);
    void lightsUpdateFromTerminal(const std::vector<LightControllerSettings> &lightSettings);
    void handleLightsRetrieve(const LightsRetrieveMessage &message, QHostAddress fromAddr);
    void handleRemoteLights(int lightId);

    Components* mSystemComponents;
public slots:
    void handleStateChanged(PinIdentifier pinId, bool state);
    void handleHardwareReprovisionNotif(ControllerInfo controller);

private:       

    std::map<int,int> mLightToGroupingMap;
    std::mutex mDbMutex;
};

