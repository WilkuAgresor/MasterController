#pragma once

#include <QRunnable>
#include <QObject>
#include <QDebug>
#include <QUdpSocket>
#include <QNetworkDatagram>

class MainApplication;

class ConnectionTimer : public QObject, public QRunnable
{
    Q_OBJECT
public:
    ConnectionTimer(MainApplication *sender, QString controllerName);
public:
    void run();
private:
    MainApplication* mUdpSender;
    QString mControllerName;
};
