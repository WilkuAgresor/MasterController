#pragma once

#include <QNetworkDatagram>
#include <QThreadPool>

#include <connection/connectiontimer.hpp>

class MainApplication;

struct ConnectionEntry
{
    QString      controllerName;
    QHostAddress controllerAddress;

    bool operator==(const ConnectionEntry& a) const
    {
        return (controllerName == a.controllerName);
    }
};

class SlaveConnectionMonitor
{
public:
    SlaveConnectionMonitor(MainApplication* appContext);

    /// returns true if OK, returns false if the entry exists
    bool addEntry(QString controllerName, QHostAddress controllerAddress);

    ConnectionEntry getKey(const QHostAddress& ipAddr);

    void eraseEntry(const QString& controllerName);

    bool isAvailableForSending(const QString& controllerName);

    void startTimer(const QString& controllerName);

private:
    std::mutex mMutex;
    std::vector<ConnectionEntry> entries;
    MainApplication* mAppContext;
    std::unique_ptr<QThreadPool> mTimerThreadPool;
};
