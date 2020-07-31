#include "SerialConnection.hpp"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <components.hpp>

SerialConnection::SerialConnection(Components *components)
    : QObject(components)
    , mComponents(components)
{
    mPort = new QSerialPort(this);
    qDebug() <<"serial conn constructor";

    auto db = DatabaseFactory::createDatabaseConnection("serial");
    for(auto& controller: db->getControllers())
    {
        if(controller.type == ControllerInfo::Type::USB_SERIAL_GRAND_CENTRAL)
        {
            mSerialDevice = controller.ipAddr;
            break;
        }
    }

    if(!connect())
    {

    }
}

bool SerialConnection::connect()
{
    auto ports = QSerialPortInfo::availablePorts();
    for(auto& port: ports)
    {
        qDebug() <<"port name: "<<port.portName() << " Serial number: "<<port.serialNumber();

        if(port.serialNumber() == mSerialDevice)
        {
            mPort->setPort(port);
            mPort->setBaudRate(mBaudRate, QSerialPort::AllDirections);
            if(mPort->open(QIODevice::ReadWrite))
            {
                QObject::connect(mPort, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
                QObject::connect(mPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
                qDebug() << "Serial connection with GrandCentral established";
                return true;
            }
            else
            {
                qDebug() << "Serial connection with GrandCentral FAILED!!!";
            }
            break;
        }
    }
    return false;
}

bool SerialConnection::sendCommand(const QString &command)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    if(!mPort->isWritable())
    {
        qDebug() << "Serial port is not writable";
        return false;
    }

    const QByteArray requestData = command.toUtf8();
    mPort->write(requestData);
    if (mPort->waitForBytesWritten())
    {
        qDebug() <<" message sent";
        return true;
    }
    qDebug() <<" message timeout";
    return false;
}

void SerialConnection::handleIncomingCommand(QString command)
try
{
    qDebug() << "incoming command: "<<command;

    auto args = command.split(",", QString::SkipEmptyParts);

    qDebug() << "args: "<<args;

    auto commandType = args.at(0);
    if(commandType == "click")
    {
        PinIdentifier pinId;
        pinId.mExpanderId = args.at(1).toInt();
        pinId.mPinId = args.at(2).toInt();

        auto database = DatabaseFactory::createDatabaseConnection("Serial");

        auto id = database->getLightIdFromPinId(pinId);

        if(id == 0)
        {
            qDebug() << "no mapping for: "<<pinId.toStringDatabase();
            return;
        }

        mComponents->mLightsComponents->handleClicked(id);
    }
    else if(commandType == "info")
    {
        QString command;
        bool first = true;
        for(auto& part: args)
        {
            if(first)
            {
                first = false;
                continue;
            }
            command.append(part);
        }
        qDebug() << "command";
    }
    else if(commandType == "stateChange")
    {
        qDebug() << "state change";
        PinIdentifier pinId;
        pinId.mExpanderId = args.at(1).toInt();
        pinId.mPinId = args.at(2).toInt();

        int state = args.at(3).toInt();

        mComponents->mGrandCentral->stateChangeNotif(pinId, state != 0);
    }
}
catch(const std::exception& ex)
{
    qDebug() << "exception during processing of incoming GrandCentral command: "<<command;
}

void SerialConnection::setInputMapping(int expanderId, std::uint16_t mapping)
{
    QString command;
    command.append("<");
    command.append("setInMap");
    command.append(",");
    command.append(QString::number(expanderId));
    command.append(",");
    command.append(QString::number(mapping));
    command.append(">");

    sendCommand(command);
}

void SerialConnection::setOutputMapping(int expanderId, std::uint16_t mapping)
{
    QString command;
    command.append("<");
    command.append("setOutMap");
    command.append(",");
    command.append(QString::number(expanderId));
    command.append(",");
    command.append(QString::number(mapping));
    command.append(">");

    sendCommand(command);
}

void SerialConnection::setGroupId(const PinIdentifier &input, std::uint16_t groupId)
{
    QString command;
    command.append("<");
    command.append("setGroupId");
    command.append(",");
    command.append(input.toStringSerial());
    command.append(",");
    command.append(QString::number(groupId));
    command.append(">");

    qDebug() << "command: "<<command;

    sendCommand(command);
}

void SerialConnection::addInputMapping(const PinIdentifier &input, const PinIdentifier &output)
{
    QString command;
    command.append("<");
    command.append("addInMap");
    command.append(",");
    command.append(input.toStringSerial());
    command.append(",");
    command.append(output.toStringSerial());
    command.append(">");

    qDebug() << "command: "<<command;

    sendCommand(command);
}

void SerialConnection::removeInputMapping(const PinIdentifier &input, const PinIdentifier &output)
{
    QString command;
    command.append("<");
    command.append("remInMap");
    command.append(",");
    command.append(input.toStringSerial());
    command.append(",");
    command.append(output.toStringSerial());
    command.append(">");

    sendCommand(command);
}

void SerialConnection::setNotifClick(const PinIdentifier &input, bool value)
{
    int val = value ? 1:0;

    QString command;
    command.append("<");
    command.append("notifClick");
    command.append(",");
    command.append(input.toStringSerial());
    command.append(",");
    command.append(QString::number(val));
    command.append(">");

    sendCommand(command);
}

void SerialConnection::setNotifDoubleClick(const PinIdentifier &input, bool value)
{
    int val = value ? 1:0;

    QString command;
    command.append("<");
    command.append("notifDClick");
    command.append(",");
    command.append(input.toStringSerial());
    command.append(",");
    command.append(QString::number(val));
    command.append(">");

    sendCommand(command);
}

void SerialConnection::setNotifPress(const PinIdentifier &input, bool value)
{
    int val = value ? 1:0;

    QString command;
    command.append("<");
    command.append("notifPress");
    command.append(",");
    command.append(input.toStringSerial());
    command.append(",");
    command.append(QString::number(val));
    command.append(">");

    sendCommand(command);
}

void SerialConnection::initAll()
{
    sendCommand("<initAll>");
}

void SerialConnection::terminateAll()
{
    sendCommand("<termAll>");
}

void SerialConnection::setInputState(const PinIdentifier &input, LogicState state)
{
    qDebug() << "set admin state: "<< input.print();

    QString command;
    command.append("<");
    command.append("adminChange,");
    command.append(input.toStringSerial());
    command.append(",");

    if(state == LogicState::ON)
    {
        command.append("1>");
    }
    else
    {
        command.append("0>");
    }

    sendCommand(command);
}

void SerialConnection::setDefaultOutputState(const PinIdentifier &output, bool value)
{
    int val = value ? 1:0;

    QString command;
    command.append("<");
    command.append("setDefOut");
    command.append(",");
    command.append(output.toStringSerial());
    command.append(",");
    command.append(QString::number(val));
    command.append(">");

    sendCommand(command);
}

void SerialConnection::flashInit()
{
    sendCommand("<flashInit>");
}

void SerialConnection::flashSave()
{
    sendCommand("<flashSave>");
}

void SerialConnection::flashRestore()
{
    sendCommand("<flashRestore>");
}

void SerialConnection::flashErase()
{
    sendCommand("<flashErase>");
}

void SerialConnection::reprovisionAllOutputStates()
{
    sendCommand("<reprov>");
}

void SerialConnection::handleReadyRead()
{
    qDebug() << "ready read";
    if(mPort->canReadLine())
    {
        QString data = QString::fromStdString(mPort->readLine().toStdString());

        auto messages = data.split("<", QString::SkipEmptyParts);

        for(auto& message: messages)
        {
           if(message.endsWith(">\r\n"))
           {
               message.chop(3);
           }
           else if(message.endsWith(">"))
           {
               message.chop(1);
           }

            handleIncomingCommand(message);
        }
    }
}

void SerialConnection::handleError(QSerialPort::SerialPortError serialPortError)
{
    qDebug() << "error occured: "<<serialPortError;
    mPort->disconnect();
    mPort = new QSerialPort(this);

    while(!connect())
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}






//QString currentRequest = "<setHigh,0,11>";

