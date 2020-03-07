#include "SerialConnection.hpp"
#include <QDebug>

SerialConnection::SerialConnection()
{
    auto ports = QSerialPortInfo::availablePorts();
    for(auto& port: ports)
    {
        qDebug() << "serial port: "<<port.portName() << " path: "<<port.systemLocation();
    }

}
