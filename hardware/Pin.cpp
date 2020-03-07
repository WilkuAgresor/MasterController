#include "Pin.hpp"

Pin::Pin()
{

}

void Pin::setPinType(PinType type)
{
    mType = type;
}

void Pin::setOutputState(OutputState state)
{
    if(mType == PinType::OUTPUT)
    {
        mOutputState = state;
    }
    else
    {
        qDebug() << "PIN is not output";
    }
}
