#include "LightsAppComponents.hpp"
#include <components.hpp>

#include <../common/messages/replyMessage.hpp>
#include <LeonardoIpExecutor/LeoMessage.hpp>

LightsAppComponents::LightsAppComponents(QObject *parent, Components *components)
    : QObject(parent)
    , mSystemComponents(components)
{
    {
        std::unique_lock<std::mutex> lock(mDbMutex);
        auto database = DatabaseFactory::createDatabaseConnection("lights");
        mLightToGroupingMap = database->getLightsGroupingMap();
        lock.unlock();

        QObject::connect(mSystemComponents->mGrandCentral, SIGNAL(inputPinStateChangeNotif(PinIdentifier, bool)),
                         this, SLOT(handleStateChanged(PinIdentifier, bool)));
        QObject::connect(mSystemComponents, SIGNAL(hardwareReprovisionNotif(ControllerInfo)), this, SLOT(handleHardwareReprovisionNotif(ControllerInfo)));
    }

//    while(!mSystemComponents->mGrandCentral->isInitialized())
//    {
//        qDebug() << "Waiting for hardware initialization";
//        std::this_thread::sleep_for(std::chrono::seconds(2));
//    }
//    reprovisionHardwareState();
}

void LightsAppComponents::reprovisionTerminalData(QHostAddress terminalAddr)
{
    LightSettingsPayload settingsPayload;
    std::unique_lock<std::mutex> lock(mDbMutex);

    auto database = DatabaseFactory::createDatabaseConnection("lights");

    auto lightSettings = database->getLightSettings();
    database.reset();
    lock.unlock();

    settingsPayload.mLightControllers = lightSettings;

    LightSettingsMessage message(settingsPayload);
//    qDebug() <<"lights reprovision settings: " << settingsPayload.toString();

    mSystemComponents->mSender->send(terminalAddr, TERMINAL_LISTEN_PORT, message.toData());
}

void LightsAppComponents::reprovisionHardwareState()
{
    std::lock_guard<std::mutex> _lock(mDbMutex);

    auto database = DatabaseFactory::createDatabaseConnection("lights");
    auto settings = database->getLightSettings();


    for(auto& light: settings)
    {
        if(light.mIsOnChanged)
        {
            LogicState state = boolToLogicState(light.mIsOn); /*light.mIsOn ? LogicState::ON : LogicState::OFF;*/

            mSystemComponents->mGrandCentral->setInputState(database->getLightGroupingId(light.mId), state);
        }
    }
}

void LightsAppComponents::handleMessage(const Message &message, QHostAddress fromAddr)
{
    auto header = message.getHeader();

    switch(header.getType())
    {
    case MessageType::LIGHTS_UPDATE:
        handleLightsUpdateFromTerminal(static_cast<const LightSettingsMessage&>(message), fromAddr);
        break;

        default:
        qDebug() << "received unknown Lights message";
    }
}

void LightsAppComponents::handleStateChanged(PinIdentifier pinId, bool state)
{
    qDebug() << "grouping state change: "<< pinId.print()<<" state: "<<state;

    //the pinId is the input pin that has triggered the change
    //deduce the grouping ID and forward this change to the terminals

    int pinGroupId = mSystemComponents->mGrandCentral->getInputPinsGroupingId(pinId);

    if(pinGroupId == -1)
    {
        qDebug() << "State change on pin: " <<pinId.print() <<" without grouping assignment. Ignoring...";
        return;
    }

    auto currentLogicState = boolToLogicState(state);


    for(const auto& pair: mLightToGroupingMap)
    {
        if(pair.second == pinGroupId)
        {
            bool stateChanged = false;
            qDebug() << "output pin of light id: "<<pair.first;
            LightControllerSettings dbSettings;

            {
                std::lock_guard<std::mutex> _lock(mDbMutex);

                auto database = DatabaseFactory::createDatabaseConnection("lights");
                dbSettings = database->getLightSetting(pair.first);

                qDebug() << "db state: "<<dbSettings.mIsOn;
                auto dbLogicState = boolToLogicState(dbSettings.mIsOn);

                if(currentLogicState != dbLogicState)
                {
                    stateChanged = true;
                    qDebug() << "STATE CHANGE!";
                    database->setLightsIsOn(pair.first, logicStateToBool(currentLogicState));
                }

                LightControllerSettings settings;
                settings.mId = pair.first;
                settings.setIsOn(state);

                LightSettingsPayload payload;
                payload.mLightControllers.push_back(settings);
                LightSettingsMessage message(payload);
                QHostAddress address;

                sendToTerminals(message, address);
            }

            if(stateChanged)
            {
                qDebug() << "remote light state change!";
                handleRemoteLights(pair.first);
            }

            break;
        }
    }
}

void LightsAppComponents::handleHardwareReprovisionNotif(ControllerInfo controllerInfo)
{
    std::lock_guard<std::mutex> _lock(mDbMutex);

    auto database = DatabaseFactory::createDatabaseConnection("lights");

    auto lightSettings = database->getLightSettings();

    switch(controllerInfo.type)
    {
    case ControllerInfo::Type::USB_SERIAL_GRAND_CENTRAL:
        for(auto& light: lightSettings)
        {
            LogicState state = boolToLogicState(light.mIsOn); /*light.mIsOn ? LogicState::ON : LogicState::OFF;*/

            mSystemComponents->mGrandCentral->setInputState(database->getLightGroupingId(light.mId), state);
        }
        break;
    case ControllerInfo::Type::ARD_LEO:
        database->updateControllerCurKey(controllerInfo.name, controllerInfo.key);
        qDebug() << "ARD LEO reprovision incoming: "<<controllerInfo.ipAddr<<" sessionId: "<<controllerInfo.key;               

        break;
    default:
        qDebug() << "Unsupported Hardware Reprovision type";
        break;
    }
}

void LightsAppComponents::handleClicked(int /*lightId*/)
{ 
    return;
}

void LightsAppComponents::sendToTerminals(const LightSettingsMessage &message, QHostAddress fromAddr)
{
    for(auto& controller: mSystemComponents->mControllers)
    {
        if(controller.type == ControllerInfo::Type::TERMINALv1)
        {
            auto address = QHostAddress(controller.ipAddr);
            if(address != fromAddr)
            {
                qDebug() << "sending "<<message.toString()<<" to "<<controller.ipAddr;
                mSystemComponents->mSender->send(address, TERMINAL_LISTEN_PORT, message.toData());
            }
        }        
    }
}

void LightsAppComponents::handleLightsUpdateFromTerminal(const LightSettingsMessage &message, QHostAddress fromAddr)
{
    qDebug() << "handle lights update";

    lightsUpdateFromTerminal(message.payload().mLightControllers);

    auto header = message.getHeader();

    if(header.mExpectReply)
    {
        ReplyPayload replyPayload(Status::OK);
        ReplyMessage replyMessage(replyPayload);

        qDebug() << "sending reply: "<<replyMessage.toString() <<" to: "<<fromAddr.toString() <<":"<<header.mReplyPort;
        mSystemComponents->mSender->send(fromAddr, header.mReplyPort, replyMessage.toData());
    }
}

void LightsAppComponents::lightsUpdateFromTerminal(const std::vector<LightControllerSettings> &lightSettings)
{
    std::lock_guard<std::mutex> _lock(mDbMutex);

    auto database = DatabaseFactory::createDatabaseConnection("lights");

    for(auto& light: lightSettings)
    {
        if(light.mIsOnChanged)
        {
            LogicState state = boolToLogicState(light.mIsOn); /*light.mIsOn ? LogicState::ON : LogicState::OFF;*/

            mSystemComponents->mGrandCentral->setInputState(database->getLightGroupingId(light.mId), state);
        }

        if(light.mDimmChanged && (light.mType == LightControllerType::DIMM || light.mType == LightControllerType::COLOR))
        {
            qDebug() << "setting dimm: "<<light.mDimm;
            database->setLightsDimm(light.mId, light.mDimm);
        }

        if(light.mColorChanged && light.mType == LightControllerType::COLOR)
        {
            qDebug() << "setting color: "<<light.mColor;
            database->setLightsColor(light.mId, light.mColor);
        }

        if(light.mGuiSettingsChanged)
        {
            qDebug() << "setting gui settings: "<<light.mGuiSettings.toString();
            database->setLightsGuiSettings(light.mId, light.mGuiSettings);
        }
    }
}

void LightsAppComponents::handleLightsRetrieve(const LightsRetrieveMessage &message, QHostAddress fromAddr)
{
    std::unique_lock<std::mutex> lock(mDbMutex);

    auto database = DatabaseFactory::createDatabaseConnection("lights");

    LightSettingsPayload respPayload;
    respPayload.mLightControllers = database->getLightSettings();
    database.reset();
    lock.unlock();
    LightSettingsMessage respMessage(respPayload);

    auto header = message.getHeader();
  //  qDebug() << "sending response: "<<respMessage.toString() <<" to: "<<fromAddr.toString() <<":"<<header.mReplyPort;

    mSystemComponents->mSender->send(fromAddr, header.mReplyPort, respMessage.toData());
    qDebug() << "reprov:";
    mSystemComponents->mGrandCentral->reprovisionOutputValues();
    qDebug() << "after reprov:";
}

void LightsAppComponents::handleRemoteLights(int lightId)
{
    std::unique_lock<std::mutex> lock(mDbMutex);

    LightControllerSettings dbSettings;

    auto database = DatabaseFactory::createDatabaseConnection("lights");
    dbSettings = database->getLightSetting(lightId);

    qDebug() << "light settings: "<<dbSettings.toString();

    switch (dbSettings.mType)
    {
    case LightControllerType::BASIC:
    {
        auto settings = database->getRemoteSwitchSettings(lightId);
        for(auto & setting: settings)
        {
            if(!dbSettings.mIsOn)
            {
               setting.mValue = 1; //HIGH state means OFF
            }
            else
            {
               setting.mValue = 0;
            }

            LeoSetMessage message(setting);
            qDebug() << "remote lights message: "<<QString(message.serialize()) <<" to controller: "<<setting.mControllerIpAddr <<":"<< setting.mControllerPort;

            mSystemComponents->mSender->sendRaw(QHostAddress(setting.mControllerIpAddr), setting.mControllerPort, message.serialize());
        }
    }
        break;
    case LightControllerType::DIMM:
    {
        auto settings = database->getDimmLightSettings(lightId);
        for(auto & setting: settings)
        {
            if(!dbSettings.mIsOn)
            {
                setting.mValue = 0; //value 0 is OFF on PWM
            }
            else
            {
                setting.mValue = gammaCorrect(dbSettings.mDimm);
            }

            LeoSetMessage message(setting);

            qDebug() << "remote lights message: "<<QString(message.serialize()) <<" to controller: "<<setting.mControllerIpAddr <<":"<< setting.mControllerPort;

            mSystemComponents->mSender->sendRaw(QHostAddress(setting.mControllerIpAddr), setting.mControllerPort, message.serialize());
        }
    }
        break;
    case LightControllerType::COLOR:
    {
        auto settings = database->getRGBSetting(lightId);

        if(!dbSettings.mIsOn)
        {
            qDebug() << "remote color light setting OFF";

            settings.mRedValue = 0; //value 0 is OFF for PWM
            settings.mGreenValue = 0;
            settings.mBlueValue = 0;
        }
        else
        {
            settings.setRGBFromColor(dbSettings.mColor);
        }

        for(const auto& setting: settings.getRedPins())
        {            
            LeoSetMessage message(setting);
            qDebug() << "remote lights message for RED: "<<message.serialize() <<" to controller: "<<setting.mControllerIpAddr <<":"<< setting.mControllerPort;
            mSystemComponents->mSender->sendRaw(QHostAddress(setting.mControllerIpAddr), setting.mControllerPort, message.serialize());
        }

        for(const auto& setting: settings.getGreenPins())
        {            
            LeoSetMessage message(setting);
            qDebug() << "remote lights message for GREEN: "<<message.serialize() <<" to controller: "<<setting.mControllerIpAddr <<":"<< setting.mControllerPort;
            mSystemComponents->mSender->sendRaw(QHostAddress(setting.mControllerIpAddr), setting.mControllerPort, message.serialize());
        }

        for(const auto& setting: settings.getBluePins())
        {
            LeoSetMessage message(setting);
            qDebug() << "remote lights message for BLUE: "<<message.serialize() <<" to controller: "<<setting.mControllerIpAddr <<":"<< setting.mControllerPort;
            mSystemComponents->mSender->sendRaw(QHostAddress(setting.mControllerIpAddr), setting.mControllerPort, message.serialize());
        }
    }
        break;
    }
}
