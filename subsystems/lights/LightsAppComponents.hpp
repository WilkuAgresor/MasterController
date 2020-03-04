#pragma once

#include <QNetworkDatagram>
#include <../common/subsystems/lights/LightControllerSettings.hpp>
#include <../common/subsystems/lights/LightsRetrieveMessage.hpp>
#include <../common/CommonDefinitions.hpp>
#include <../common/subsystems/lights/LightsDictionary.hpp>

class Components;

class LightsAppComponents
{
public:
    LightsAppComponents(Components* components);

    void reprovisionTerminalData(QHostAddress terminalAddr);
    void handleMessage(const Message& message, QHostAddress fromAddr, int fromPort);
    void handleLightsUpdate(const LightSettingsMessage &message, QHostAddress fromAddr, int fromPort);
    void handleLightsRetrieve(const LightsRetrieveMessage &message, QHostAddress fromAddr);

    Components* mSystemComponents;
};

