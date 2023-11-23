#include "Pin.hpp"
#include <serialListener/SerialConnection.hpp>

Pin::Pin(QObject* parent, SerialConnection *serialConnection, const PinIdentifier& pinId, PinType type)
    : QObject(parent),
      mSerialConnection(serialConnection),
      mPinId(pinId),
      mType(type)
{
    QObject::connect(this, SIGNAL(stateChange(PinIdentifier, LogicState)), mSerialConnection, SLOT(setInputState(PinIdentifier, LogicState)), Qt::AutoConnection);

    if(type == PinType::OUTPUT_LOW || type == PinType::INPUT_NO_PULLUP)
    {
        mDefaultState = OutputState::LOW;
    }
    else
    {
        mDefaultState = OutputState::HIGH;
    }
}

PinType Pin::getPinType() const
{
    return mType;
}

OutputState Pin::getDefaultState() const
{
    return mDefaultState;
}

void Pin::setLogicInputState(LogicState state)
{
    mLogicalOutputState = state;
    stateChange(mPinId, state);
}

PinIdentifier Pin::getPinId() const
{
    return mPinId;
}
