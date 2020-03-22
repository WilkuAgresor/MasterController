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


    //initialize pins
    initializePins();
    //initialize mappings
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

void GrandCentral::setOutputState(int mappingId, LogicState state)
{
    qDebug() << "state change to "<<static_cast<int>(state);

    for(auto& mapping: mMappings)
    {
        if(mapping.getId() == mappingId)
        {
            for(auto& pinId: mapping.getOutputPins())
            {
                for(auto& pin: mPins)
                {
                    if(pin->getPinId() == pinId)
                    {
                        pin->setLogicalOutputState(state);
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
    emit(outputPinStateChangeNotif(id,state));
}

int GrandCentral::getPinsGroupingId(const PinIdentifier &pin)
{
    for(auto& grouping: mMappings)
    {
        if(grouping.containsOutputPin(pin))
        {
            return grouping.getId();
        }
    }
    return -1;
}

OutputState GrandCentral::getPinDefaultOutputState(const PinIdentifier &pin)
{
    for(const auto& pinData: mPins)
    {
        if(pinData->getPinId() == pin)
        {
            return pinData->getDefaultState();
        }
    }
    return OutputState::UNDEFINED;
}

void GrandCentral::resetGrandCentralSettings()
{
}

void GrandCentral::setInOutMappings()
{
    std::array<std::bitset<16>,16> inMappings;
    std::array<std::bitset<16>,16> outMappings;

    for(auto& pin: mPins)
    {
        if(pin->getPinType() == PinType::INPUT_PULLUP || pin->getPinType() == PinType::INPUT_NO_PULLUP)
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

    for(std::uint16_t i=0; i<inMappings.size(); i++)
    {
        std::uint16_t inMapInt = inMappings[i].to_ulong();
        std::uint16_t outMapInt = outMappings[i].to_ulong();

        qDebug() << "input mapping: "<<inMapInt<<", output mapping: "<< outMapInt;

        mSerialConnection->setInputMapping(i, inMapInt);
        mSerialConnection->setOutputMapping(i, outMapInt);
    }

    for(auto& grouping: mMappings)
    {
        for(auto input: grouping.getInputPins())
        {
            for(auto output: grouping.getOutputPins())
            {
                mSerialConnection->addInputMapping(input, output);
            }
        }
    }
    mSerialConnection->initAll();
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

    mSerialConnection->flashSave();
}
