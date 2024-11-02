#include "GrandCentral.hpp"
#include <components.hpp>
#include <database/database.hpp>
#include <bitset>
#include <QtConcurrent/QtConcurrent>
#include <QFile>

GrandCentral::GrandCentral(QObject *parent, Components *components)
    : QObject(parent)
    , mComponents(components)
    , mSerialConnection(new SerialConnection(mComponents))
{
    qRegisterMetaType<PinIdentifier>("PinIdentifier");
    qRegisterMetaType<OutputState>("OutputState");
    qRegisterMetaType<LogicState>("LogicState");

    QObject::connect(mSerialConnection, SIGNAL(sessionIdUpdate(quint32)), this, SLOT(handleSessionIdChange(quint32)));

    {
        std::lock_guard<std::mutex> _lock(mDbMutex);
        auto db = DatabaseFactory::createDatabaseConnection("hardware");
        auto pinMap = db->getGrandCentralPins();

        std::ranges::transform(pinMap, std::back_inserter(mPins), [this](const auto& x){auto  pin = new Pin(this, mSerialConnection, x.first, x.second); return pin; });

        mMappings = db->getGrandCentralPinGroupings();
    }

    mSerialConnection->initialize();

//    //initialize pins
//    initializePins();
//    //initialize mappings
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

    qDebug() << "mappings size: "<<mMappings.size();

    auto setLogicInputStateForPin = [&](PinIdentifier pinId, LogicState state) {
        auto it = std::find_if(mPins.begin(), mPins.end(), [&](Pin* pin) {
            return pin->getPinId() == pinId && pin->getPinType() == PinType::VIRTUAL_INPUT;
        });

        if (it != mPins.end()) {
            qDebug() << "setting logic input state";
            (*it)->setLogicInputState(state);
        }
    };

    if(auto mapping = std::ranges::find_if(mMappings, [mappingId](const auto& x){return x.getId() == mappingId;}); mapping != mMappings.end())
    {
        for (const auto& pinId : mapping->getInputPins()) {
            setLogicInputStateForPin(pinId, state);
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

    if(auto result = std::ranges::find_if(mMappings, [&pin](const auto& x){return x.containsOutputPin(pin);}); result != mMappings.end())
    {
        return result->getId();
    }

    return -1;
}

int GrandCentral::getInputPinsGroupingId(const PinIdentifier &pin)
{
    if(!isInitialized())
    {
        return -1;
    }

    if(auto result = std::ranges::find_if(mMappings, [&pin](const auto& x){return x.containsInputPin(pin);}); result != mMappings.end())
    {
        return result->getId();
    }

    return -1;
}

OutputState GrandCentral::getPinDefaultOutputState(const PinIdentifier &pin)
{
    if(!isInitialized())
    {
        return OutputState::UNDEFINED;
    }

    if(auto result = std::ranges::find_if(mPins, [&pin](const auto x){return x->getPinId()  == pin;}); result != mPins.end())
    {
        return (*result)->getDefaultState();
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
    qDebug() << "grand central initialized: "<< mIsInitialized;
    return mIsInitialized;
}

void GrandCentral::handleSessionIdChange(quint32 sessionId)
{
    qDebug() << "sessionId update: "<<sessionId;

    if(sessionId == 0)
    {
        return;
    }

    std::unique_lock<std::mutex> lock(mDbMutex);


    auto db = DatabaseFactory::createDatabaseConnection("hardware");

    auto controllers = db->getControllers();

    auto grandCentralControllerInfo = std::ranges::find_if(controllers, [](const auto& x){return x.type == ControllerInfo::Type::USB_SERIAL_GRAND_CENTRAL;});

    if(grandCentralControllerInfo == controllers.end())
    {
        throw("Didn't find Grand Central controller in the database. This one is mandatory");
    }

    auto forceControllerUpdate = QFile::exists(sForceControllerConfigurationFlagFile);

    qDebug() << "force controller update value: "<<forceControllerUpdate;

    if(sessionId != grandCentralControllerInfo->key || forceControllerUpdate)
    {
        //grand central was rebooted. Need to send down the current configuration and update the sessionID in the Database
        qDebug() << "new hardware session ID. Saving in the DB";
        db->updateControllerCurKey(grandCentralControllerInfo->name, sessionId);
        grandCentralControllerInfo->key = sessionId;
        db.reset();
        lock.unlock();

        initializePins();
        mComponents->sendHardwareReprovisionNotif(*grandCentralControllerInfo);
    }
    else
    {
        lock.unlock();
        qDebug() << "Known session ID, reprovisioning the DB data";

        mSerialConnection->terminateAll();
        setInOutMappings();
        mSerialConnection->initAll();

        mComponents->sendHardwareReprovisionNotif(*grandCentralControllerInfo);
    }
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
        const auto pinType = pin->getPinType();
        const auto pinId = pin->getPinId();

        if(pinType == PinType::INPUT_PULLUP || pinType == PinType::INPUT_NO_PULLUP || pinType == PinType::VIRTUAL_INPUT
            || pinType == PinType::INPUT_PULLUP_MIRROR || pinType == PinType::INPUT_NO_PULLUP_MIRROR)
        {
            inMappings[pin->getPinId().mExpanderId][pinId.mPinId] = 1;
        }
        else if(pinType == PinType::OUTPUT_LOW || pinType == PinType::OUTPUT_HIGH)
        {
            outMappings[pin->getPinId().mExpanderId][pinId.mPinId] = 1;
        }

        if(pinType == PinType::INPUT_PULLUP || pinType == PinType::INPUT_PULLUP_MIRROR || pinType == PinType::OUTPUT_HIGH)
        {
            mSerialConnection->setDefaultOutputState(pinId, true);
        }

        if(pinType == PinType::INPUT_PULLUP_MIRROR || pinType == PinType::INPUT_NO_PULLUP_MIRROR)
        {
            mSerialConnection->setInputPinTypeMirror(pinId);
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
    mSerialConnection->flashErase();

    mSerialConnection->terminateAll();

    setInOutMappings();

    mSerialConnection->initAll();

    mSerialConnection->flashSave();
}
