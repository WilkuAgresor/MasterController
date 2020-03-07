#pragma once

#include <QSerialPort>
#include <QSerialPortInfo>


class SerialConnection
{
public:
    SerialConnection();

    QString mSerialDevice = "ttyACM0";
};
