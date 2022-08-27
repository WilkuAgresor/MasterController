#include "GrandCentral.hpp"
#include <components.hpp>
#include <database/database.hpp>
#include <bitset>

GrandCentral::GrandCentral(QObject *parent, Components *components)
    : QObject(parent)
    , mComponents(components)
    , mSerialConnection(new SerialConnection(mComponents))
{
    qRegisterMetaType<PinIdentifier>("PinIdentifier");
    qRegisterMetaType<OutputState>("OutputState");
    qRegisterMetaType<LogicState>("LogicState");


    //initialize pins
    initializePins();
    //initialize mappings
}



void GrandCentral::addOrUpdatePinMapping(const PinMapping &mapping)
{
    if(!isInitialized())
    {
        return;
    }

    auto found = std::find(mMappings.begin(), mMappings.end(), mapping);
    if(found != mMappings.end())
    {
        //update
        mMappings.erase(found);
    }
    //add
    mMappings.push_back(mapping);
}

void GrandCentral::setInputState(int mappingId, LogicState state)
{
    if(!isInitialized())
    {
        return;
    }
    qDebug() << "state change to "<<static_cast<int>(state);

    for(auto& mapping: mMappings)
    {
        if(mapping.getId() == mappingId)
        {
            for(auto& pinId: mapping.getInputPins())
            {
                for(auto& physicalPin: mPins)
                {
                    if(physicalPin->getPinId() == pinId && physicalPin->getPinType() == PinType::VIRTUAL_INPUT)
                    {
                        physicalPin->setLogicInputState(state);
                        break;
                    }
                }
            }

            break;
        }
    }
}

void GrandCentral::stateChangeNotif(PinIdentifier id, bool state)
{
    emit(inputPinStateChangeNotif(id,state));
}

int GrandCentral::getPinsGroupingId(const PinIdentifier &pin)
{
    if(!isInitialized())
    {
        return -1;
    }

    for(auto& grouping: mMappings)
    {
        if(grouping.containsOutputPin(pin))
        {
            return grouping.getId();
        }
    }
    return -1;
}

int GrandCentral::getInputPinsGroupingId(const PinIdentifier &pin)
{
    if(!isInitialized())
    {
        return -1;
    }

    for(auto& grouping: mMappings)
    {
        if(grouping.containsInputPin(pin))
        {
            return grouping.getId();
        }
    }
    return -1;
}

OutputState GrandCentral::getPinDefaultOutputState(const PinIdentifier &pin)
{
    if(!isInitialized())
    {
        return OutputState::UNDEFINED;
    }

    for(const auto& pinData: mPins)
    {
        if(pinData->getPinId() == pin)
        {
            return pinData->getDefaultState();
        }
    }
    return OutputState::UNDEFINED;
}

void GrandCentral::reprovisionOutputValues()
{
    if(!isInitialized())
    {
        return;
    }

    mSerialConnection->reprovisionAllOutputStates();
}

void GrandCentral::setInitialized(bool value)
{
    mIsInitialized = value;
}

bool GrandCentral::isInitialized()
{
    return mIsInitialized;
}

void GrandCentral::resetGrandCentralSettings()
{
}

void GrandCentral::setInOutMappings()
{   
    std::array<std::bitset<16>,26> inMappings; //also for virtual mappings
    std::array<std::bitset<16>,16> outMappings;

    for(auto& pin: mPins)
    {
        if(pin->getPinType() == PinType::INPUT_PULLUP || pin->getPinType() == PinType::INPUT_NO_PULLUP || pin->getPinType() == PinType::VIRTUAL_INPUT)
        {
            inMappings[pin->getPinId().mExpanderId][pin->getPinId().mPinId] = 1;
        }
        else if(pin->getPinType() == PinType::OUTPUT_LOW || pin->getPinType() == PinType::OUTPUT_HIGH)
        {
            outMappings[pin->getPinId().mExpanderId][pin->getPinId().mPinId] = 1;
        }

        if(pin->getPinType() == PinType::INPUT_PULLUP || pin->getPinType() == PinType::OUTPUT_HIGH)
        {
            mSerialConnection->setDefaultOutputState(pin->getPinId(), true);
        }
    }

    //set input mappings
    for(std::uint16_t i=0; i<inMappings.size(); i++)
    {
        std::uint16_t inMapInt = inMappings[i].to_ulong();
        mSerialConnection->setInputMapping(i, inMapInt);
    }

    //set output mappings
    for(std::uint16_t i=0; i<outMappings.size(); i++)
    {
        std::uint16_t outMapInt = outMappings[i].to_ulong();
        mSerialConnection->setOutputMapping(i, outMapInt);
    }

    for(auto& grouping: mMappings)
    {
        for(auto input: grouping.getInputPins())
        {
            for(auto output: grouping.getOutputPins())
            {
                mSerialConnection->addInputMapping(input, output);
                mSerialConnection->setGroupId(input, grouping.getId());
            }
        }
    }
}

void GrandCentral::initializePins()
{
    {
        auto db = DatabaseFactory::createDatabaseConnection("hardware");
        auto pinMap = db->getGrandCentralPins();

        for(auto& pair: pinMap)
        {           
            mPins.emplace_back(new Pin(this, mSerialConnection, pair.first, pair.second));
        }
        mMappings = db->getGrandCentralPinGroupings();
    }

    mSerialConnection->flashErase();

    mSerialConnection->terminateAll();

    setInOutMappings();

    mSerialConnection->initAll();

    mSerialConnection->flashSave();
}
