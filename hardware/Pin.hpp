#pragma once

#include <hardware/PinIdentifier.hpp>
#include <QDebug>

enum class PinType : int
{
    UNDEFINED = 0,
    INPUT,
    OUTPUT
};

enum class OutputState : int
{
    UNDEFINED = 0,
    LOW,
    HIGH
};

class Pin
{
public:
    Pin();

    PinType getPinType();
    void setPinType(PinType type);

    void setOutputState(OutputState state);

private:
    PinType mType = PinType::UNDEFINED;
    OutputState mOutputState = OutputState::UNDEFINED;
    int mMappingId;
};
