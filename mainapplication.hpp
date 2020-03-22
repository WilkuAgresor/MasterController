#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QDebug>
#include <mutex>

#include <database/database.hpp>
#include <connection/connectionmonitor.hpp>
#include <components.hpp>
#include <terminalListener/terminalListener.hpp>
#include <subsystems/heating/HeatingApp.hpp>
#include <subsystems/topology/TopologyApp.hpp>
#include <subsystems/lights/LightsApp.hpp>

#include <../common/simplecrypt/simplecrypt.hpp>
#include <../common/messages/message_old.hpp>
#include <../common/receiver/receiver.hpp>

#include <serialListener/SerialConnection.hpp>

enum class ConnectionState
{
    NO_CONNECTION,
    IN_PROGRESS,
    SUCCESS,
    FAILURE,
    TIME_OUT
};

class MainApplication : public QObject
{
    Q_OBJECT
public:
    explicit MainApplication(QObject *parent = nullptr);
    ~MainApplication() = default;

    void changeConnectionState(QString controllerName, ConnectionState state);

signals:
    void inProgress(const QString& controllerName);
    void finished(const QString& controllerName, const QString& opStatus);
    void resourceBusy();

private:
    std::uint64_t generateNewKey();
    SlaveConnectionMonitor mKeyKeeper;

    std::unique_ptr<Components> mComponents;
    HeatingApp * mHeatingApp;
    TopologyApp* mTopologyApp;
    TerminalListener* mTerminalListener;
    LightsApp* mLightsApp;
};

