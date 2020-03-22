#pragma once

#include <QRunnable>
#include <QObject>
#include <QDebug>
#include <QUdpSocket>
#include <QNetworkDatagram>

#include <components.hpp>

class MessageHandler : public QObject, public QRunnable
{
    Q_OBJECT
public:
    MessageHandler(QObject* parent, QNetworkDatagram datagram, Components* components);
    ~MessageHandler() = default;
signals:
    void result(QNetworkDatagram datagram);
protected:
    void run();
private:
    QNetworkDatagram mDatagram;
    Components* mComponents;
    static std::mutex mMutex;
};

