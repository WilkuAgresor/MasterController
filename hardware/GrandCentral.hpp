#pragma once

#include <QObject>
#include <hardware/Pin.hpp>
#include <hardware/PinMapping.hpp>

class GrandCentral
{
public:
    GrandCentral();

    void setPinType(PinIdentifier id, PinType type);
    void addOrUpdatePinMapping(const PinMapping& mapping);

    void setOutputState(int mappingId, OutputState state);

private:
    std::map<PinIdentifier, Pin> mPins;
    std::vector<PinMapping> mMappings;
};
