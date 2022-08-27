#pragma once
#include <mutex>

#include <QNetworkDatagram>
#include <QTimer>

#include <../common/subsystems/schedule/ScheduleEventTypes.hpp>
#include <../common/CommonDefinitions.hpp>

class Components;

class ScheduleAppComponents
{
public:
    ScheduleAppComponents(Components* components);

    void retrieveAllData(QHostAddress terminalAddr, quint16 port);
    void reprovisionTerminalData(QHostAddress terminalAddr);

    void handleMessage(const Message& message, QHostAddress fromAddr);

private:



    Components* mSystemComponents;

    std::mutex mDbMutex;
};

