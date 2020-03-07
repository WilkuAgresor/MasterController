#include "GrandCentral.hpp"

GrandCentral::GrandCentral()
{

}

void GrandCentral::setPinType(PinIdentifier id, PinType type)
try
{
    mPins.at(id).setPinType(type);
}
catch (...) {
qDebug() << "Not found Pin " << id.mExpanderId << "/" << id.mPinId;
}


void GrandCentral::addOrUpdatePinMapping(const PinMapping &mapping)
{
    auto found = std::find(mMappings.begin(), mMappings.end(), mapping);
    if(found != mMappings.end())
    {
        //update
        mMappings.erase(found);
    }
    //add
    mMappings.push_back(mapping);
}

void GrandCentral::setOutputState(int mappingId, OutputState state)
{
    qDebug() << "state change to "<<static_cast<int>(state);

    for(auto& mapping: mMappings)
    {
        if(mapping.getId() == mappingId)
        {
            for(auto& pin: mapping.getOutputPins())
            {
                pin.print();
                mPins[pin].setOutputState(state);
            }
            break;
        }
    }
}
