#include "PinIdentifier.hpp"

bool outputStateToBool(OutputState state)
{
    if(state == OutputState::HIGH)
    {
        return true;
    }
    return false;
}

OutputState boolToOutputState(bool state)
{
    return state?OutputState::HIGH : OutputState::LOW;
}

bool logicStateToBool(LogicState state)
{
    if(state == LogicState::ON)
    {
        return true;
    }
    return false;
}

LogicState boolToLogicState(bool state)
{
    return state ? LogicState::ON : LogicState::OFF;
}
