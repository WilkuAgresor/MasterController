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

    bool initialize();

    bool sendCommand(const QString& command);
    void handleIncomingCommand(QString command);

    void setInputMapping(int expanderId, std::uint16_t mapping);
    void setGroupId(const PinIdentifier &input, std::uint16_t groupId);

    bool getSessionId();

    void setOutputMapping(int expanderId, std::uint16_t mapping);

    void addInputMapping(const PinIdentifier& input, const PinIdentifier& output);
    void removeInputMapping(const PinIdentifier& input, const PinIdentifier& output);

    void setNotifClick(const PinIdentifier& input, bool click);
    void setNotifDoubleClick(const PinIdentifier& input, bool value);
    void setNotifPress(const PinIdentifier& input, bool press);
    void initAll();
    void terminateAll();

    void setDefaultOutputState(const PinIdentifier& output, bool value);
    void setInputPinTypeMirror(const PinIdentifier &output);

    void flashInit();
    void flashSave();
    void flashRestore();
    void flashErase();

    void reprovisionAllOutputStates();

    bool connect();
signals:
    void sessionIdUpdate(quint32 sessionId);
    void serialDisconnected();
    void serialConnected();
public slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError serialPortError);
    void setInputState(const PinIdentifier& input, LogicState state);

private:
    uint32_t mSessionId = 0;
    std::mutex mSendMutex;
    std::mutex mReceiveMutex;
    Components* mComponents;
    QString mSerialDevice;
    int mBaudRate = 74880;
    std::unique_ptr<QSerialPort> mPort;
};
