#include "Pin.hpp"
#include <serialListener/SerialConnection.hpp>

Pin::Pin(QObject* parent, SerialConnection *serialConnection, const PinIdentifier& pinId, PinType type)
    : QObject(parent),
      mSerialConnection(serialConnection),
      mPinId(pinId),
      mType(type)
{
    QObject::connect(this, SIGNAL(stateChange(PinIdentifier, LogicState)), mSerialConnection, SLOT(setInputState(PinIdentifier, LogicState)));

    if(type == PinType::OUTPUT_LOW || type == PinType::INPUT_NO_PULLUP)
    {
        mDefaultState = OutputState::LOW;
    }
    else
    {
        mDefaultState = OutputState::HIGH;
    }
}

//Pin::Pin(const Pin& pin)
//    :QObject(pin.parent()),
//    mSerialConnection(pin.mSerialConnection),
//    mType(pin.mType),
//    mLogicalOutputState(pin.mLogicalOutputState),
//    mDefaultState(pin.mDefaultState),
//    mMappingId(pin.mMappingId)
//{
//    QObject::connect(this, SIGNAL(stateChange(PinIdentifier, OutputState)), &mSerialConnection, SLOT(setOutputState(PinIdentifier, OutputState)));
//}

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

    emit(stateChange(mPinId, state));
}

PinIdentifier Pin::getPinId() const
{
    return mPinId;
}
