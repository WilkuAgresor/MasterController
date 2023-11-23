#pragma once

#include <QString>
#include <QHostAddress>

const QString sForceControllerConfigurationFlagFile("/tmp/forceConfigurationToControllers");

struct ControllerInfo
{
    enum Type : int
    {
        RPI_3Bp = 0,
        ARD_LEO = 1,
        TERMINALv1 = 2,
        USB_SERIAL_START = 1000,
        USB_SERIAL_GRAND_CENTRAL = 1001
    };

    enum Status : int
    {
        INACTIVE = 0,
        ACTIVE   = 1
    };

    ControllerInfo() = default;
    ~ControllerInfo() = default;

    ControllerInfo(const ControllerInfo& copy)
    {
        name = copy.name;
        ipAddr = copy.ipAddr;
        port = copy.port;
        key = copy.key;
        type = copy.type;
        status = copy.status;
    }

    QString name;
    QString ipAddr;
    uint64_t port;
    uint64_t key;
    Type type;
    Status status;

    QString print()
    {
        return "name="+name+" ipAddr="+ ipAddr+" port="+ QString::number(port)+" key="+QString::number(key) + " type="+QString::number(static_cast<int>(type))
                 + " status="+QString::number(static_cast<int>(status));
    }

    QHostAddress getIpAddress()
    {
        return QHostAddress(ipAddr);
    }
};

Q_DECLARE_METATYPE(ControllerInfo);
