#pragma once

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QObject>
#include <mutex>
#include <hardware/PinIdentifier.hpp>

class Components;

class SerialConnection : public QObject
{
    Q_OBJECT
public:
    SerialConnection(Components* components);
    ~SerialConnection() = default;

    bool sendCommand(const QString& command);
    void handleIncomingCommand(QString command);

    void setInputMapping(int expanderId, std::uint16_t mapping);
    void setOutputMapping(int expanderId, std::uint16_t mapping);

    void addInputMapping(const PinIdentifier& input, const PinIdentifier& output);
    void removeInputMapping(const PinIdentifier& input, const PinIdentifier& output);

    void setNotifClick(const PinIdentifier& input, bool click);
    void setNotifDoubleClick(const PinIdentifier& input, bool value);
    void setNotifPress(const PinIdentifier& input, bool press);
    void initAll();
    void terminateAll();

    void setDefaultOutputState(const PinIdentifier& output, bool value);

    void flashInit();
    void flashSave();
    void flashRestore();
    void flashErase();

    bool connect();
public slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError serialPortError);
    void setOutputState(const PinIdentifier& input, OutputState state);

private:
    std::mutex mMutex;
    Components* mComponents;
    QString mSerialDevice;
    int mBaudRate = 74880;
    QSerialPort* mPort;
};
