#pragma once

#include <hardware/PinIdentifier.hpp>
#include <QDebug>

class SerialConnection;

class Pin : public QObject
{
    Q_OBJECT
public:
    Pin(QObject* parent, SerialConnection* serialConnection, const PinIdentifier& pinId, PinType type = PinType::UNUSED);
//    Pin (const Pin &pin);

    PinType getPinType() const;
    OutputState getDefaultState() const;
    void setLogicInputState(LogicState state);
    PinIdentifier getPinId() const;

    friend bool operator== (const Pin &c1, const Pin &c2)
    {
        return c1.getPinId() == c2.getPinId();
    }
    friend bool operator!= (const Pin &c1, const Pin &c2)
    {
        return c1.getPinId() != c2.getPinId();
    }
    bool operator <(const Pin& rhs) const
    {
        return getPinId() < rhs.getPinId();
    }

signals:
    void stateChange(PinIdentifier id, LogicState state);

protected:
    SerialConnection* mSerialConnection;
    PinIdentifier mPinId;
    PinType mType = PinType::UNUSED;
    LogicState mLogicalOutputState = LogicState::UNDEFINED;
    OutputState mDefaultState = OutputState::UNDEFINED;
    int mMappingId;
};
