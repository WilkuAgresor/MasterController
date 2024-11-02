#include "SerialConnection.hpp"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <components.hpp>

#include <thread>
#include <chrono>

SerialConnection::SerialConnection(Components *components)
    : QObject(components)
    , mComponents(components)
{
    mPort = std::make_unique<QSerialPort>();
}

bool SerialConnection::initialize()
{
    qDebug() <<"serial conn initialization";

    {
        std::lock_guard<std::mutex> _lock(mReceiveMutex);
        auto db = DatabaseFactory::createDatabaseConnection("serial");

        auto controllers = db->getControllers();

        auto grandCentralControllerInfo = std::ranges::find_if(controllers, [](const auto& x){return x.type == ControllerInfo::Type::USB_SERIAL_GRAND_CENTRAL;});

        if(grandCentralControllerInfo == controllers.end())
        {
            throw("Didn't find Grand Central controller in the database. This one is mandatory for serial connection");
        }

        mSerialDevice = grandCentralControllerInfo->ipAddr;
    }

    int counter = 0;

    while(!connect())
    {
        if(counter >= 10)
        {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
        counter++;
    }

    return true;
}

bool SerialConnection::connect()
{
    std::unique_lock<std::mutex> lock(mReceiveMutex);

    mPort.reset(new QSerialPort());

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
                qDebug() << "serial port open";
                QObject::connect(mPort.get(), SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
                QObject::connect(mPort.get(), SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));

                qDebug() << "receive serial clear mutex";
                lock.unlock();

                auto sessionIdResult =  getSessionId();
                if(sessionIdResult)
                {
                    auto runResult = QtConcurrent::run([this]{
                        int counter = 0;
                        while(mSessionId == 0)
                        {
                            qDebug() << "Waiting for session ID with grand central";
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                            counter++;
                            if(counter >= 30)
                            {
                                qDebug() << "Failed to get sessionID of Grand Central";
                            }
                        }

                        qDebug() << "Session ID: "<< mSessionId;
                        serialConnected();
                        qDebug() << "finished processing serial connection";
                    });

                    qDebug() << "waiting is asynchronous";

                    return true;
                }                
                return false;
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
    std::lock_guard<std::mutex> _lock(mSendMutex);

    qDebug() << "sending command via serial: " << command;

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

    auto args = command.split(",", Qt::SkipEmptyParts);

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
    if(commandType == "sessionId")
    {
        auto sessionId = args.at(1).toInt();
        if(sessionId != 0)
        {
            mSessionId = sessionId;
        }
        sessionIdUpdate(mSessionId);
    }
    else if(commandType == "info")
    {
        if(args.size() == 2 && args.at(1) == "Initialized all mcp's")
        {
            mComponents->mGrandCentral->setInitialized(true);
            qDebug() << "initialized all mcp";
        }
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
    qDebug() << "receive serial clear mutex";
}
catch(const std::exception& ex)
{
    qDebug() << "exception during processing of incoming GrandCentral command: "<<command;
}

void SerialConnection::setInputMapping(int expanderId, std::uint16_t mapping)
{
    auto command = QString("<setInMap,%1,%2>").arg(QString::number(expanderId), QString::number(mapping));
    sendCommand(command);
}

void SerialConnection::setOutputMapping(int expanderId, std::uint16_t mapping)
{
    auto command = QString("<setOutMap,%1,%2>").arg(QString::number(expanderId), QString::number(mapping));
    sendCommand(command);
}

void SerialConnection::setGroupId(const PinIdentifier &input, std::uint16_t groupId)
{
    QString command = QString("<setGroupId,%1,%2>").arg(input.toStringSerial(), QString::number(groupId));
    sendCommand(command);
}

bool SerialConnection::getSessionId()
{
    QString command = "<getSessionId>";
    return sendCommand(command);
}

void SerialConnection::addInputMapping(const PinIdentifier &input, const PinIdentifier &output)
{
    auto command = QString("<addInMap,%1,%2>").arg(input.toStringSerial(), output.toStringSerial());
    sendCommand(command);
}

void SerialConnection::removeInputMapping(const PinIdentifier &input, const PinIdentifier &output)
{
    auto command = QString("<remInMap,%1,%2>").arg(input.toStringSerial(), output.toStringSerial());
    sendCommand(command);
}

void SerialConnection::setNotifClick(const PinIdentifier &input, bool value)
{
    int val = value ? 1:0;

    QString command = QString("<notifClick,%1,%2>").arg(input.toStringSerial(), QString::number(val));
    sendCommand(command);
}

void SerialConnection::setNotifDoubleClick(const PinIdentifier &input, bool value)
{
    int val = value ? 1:0;

    auto command = QString("<notifDClick,%1,%2>").arg(input.toStringSerial(), QString::number(val));
    sendCommand(command);
}

void SerialConnection::setNotifPress(const PinIdentifier &input, bool value)
{
    int val = value ? 1:0;

    auto command = QString("<notifPress,%1,%2>").arg(input.toStringSerial(), QString::number(val));
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

    QString command = QString("<adminChange,%1,%2>").arg(input.toStringSerial(), state == LogicState::ON ? "1" : "0");
    sendCommand(command);
}

void SerialConnection::setDefaultOutputState(const PinIdentifier &output, bool value)
{
    int val = value ? 1:0;

    auto command = QString("<setDefOut,%1,%2>").arg(output.toStringSerial(), QString::number(val));
    sendCommand(command);
}

void SerialConnection::setInputPinTypeMirror(const PinIdentifier& output)
{
    auto command = QString("<setInputPinType,%1,%2>").arg(output.toStringSerial(), QString::number(1));
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
    auto trimMessage = [](QString& message)
    {
        if (message.endsWith(">\r\n"))
        {
            message.chop(3); // Remove ">\r\n"
        }
        else if (message.endsWith(">"))
        {
            message.chop(1); // Remove ">"
        }
    };

    std::unique_lock<std::mutex> lock(mReceiveMutex);
    while(mPort->canReadLine())
    {
        QString data = QString::fromStdString(mPort->readLine().toStdString());

        qDebug() << "received line: "<<data;

        auto messages = data.split("<", Qt::SkipEmptyParts);

        for(auto& message: messages)
        {
            trimMessage(message);
            handleIncomingCommand(message);
        }
    }
}

void SerialConnection::handleError(QSerialPort::SerialPortError serialPortError)
{
    mSessionId = 0;
    sessionIdUpdate(mSessionId);
    serialDisconnected();

    qDebug() << "Serial error occured. Reestablishing connection: "<<serialPortError;

    while(!connect())
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    qDebug() << "serial error handling finished";
}






//QString currentRequest = "<setHigh,0,11>";

