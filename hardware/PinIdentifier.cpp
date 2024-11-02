#include "PinIdentifier.hpp"

bool outputStateToBool(OutputState state)
{
    return state == OutputState::HIGH;
}

OutputState boolToOutputState(bool state)
{
    return state ? OutputState::HIGH : OutputState::LOW;
}

bool logicStateToBool(LogicState state)
{
    return state == LogicState::ON;
}

LogicState boolToLogicState(bool state)
{
    return state ? LogicState::ON : LogicState::OFF;
}
