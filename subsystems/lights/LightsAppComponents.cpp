#include "LightsAppComponents.hpp"
#include <components.hpp>

#include <../common/messages/replyMessage.hpp>

LightsAppComponents::LightsAppComponents(QObject *parent, Components *components)
    : QObject(parent)
    , mSystemComponents(components)
{
    std::unique_lock<std::mutex> lock(mDbMutex);
    auto database = DatabaseFactory::createDatabaseConnection("lights");
    mLightToGroupingMap = database->getLightsGroupingMap();
    lock.unlock();

    QObject::connect(mSystemComponents->mGrandCentral, SIGNAL(inputPinStateChangeNotif(PinIdentifier, bool)),
                     this, SLOT(handleStateChanged(PinIdentifier, bool)));
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

void LightsAppComponents::handleMessage(const Message &message, QHostAddress fromAddr)
{
    auto header = message.getHeader();

    switch(header.getType())
    {
    case MessageType::LIGHTS_UPDATE:
        handleLightsUpdateFromTerminal(static_cast<const LightSettingsMessage&>(message), fromAddr);
        break;

//    case MessageType::HEAT_SETTINGS_RETRIEVE:
//        handleLightsRetrieve(static_cast<const LightsRetrieveMessage&>(message), fromAddr);
//        break;

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

    std::unique_lock<std::mutex> lock(mDbMutex);

    for(const auto& pair: mLightToGroupingMap)
    {
        if(pair.second == pinGroupId)
        {
            qDebug() << "output pin of light id: "<<pair.first;
            LightControllerSettings dbSettings;

            auto database = DatabaseFactory::createDatabaseConnection("lights");
            dbSettings = database->getLightSetting(pair.first);

            qDebug() << "db state: "<<dbSettings.mIsOn;
            auto dbLogicState = boolToLogicState(dbSettings.mIsOn);

            if(currentLogicState != dbLogicState)
            {
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
            break;
        }
    }
}

void LightsAppComponents::handleClicked(int /*lightId*/)
{
//    //handle click. Save state in the database.

//    std::lock_guard<std::mutex> lock(mDbMutex);

//    LightControllerSettings dbSettings;

//    auto database = DatabaseFactory::createDatabaseConnection("lights");
//    dbSettings = database->getLightSetting(lightId);
//    database->setLightsIsOn(lightId, !dbSettings.mIsOn);

//    qDebug() << "light settings: "<<dbSettings.toString();

//    settings.setIsOn(!dbSettings.mIsOn);

//    LightSettingsPayload payload;
//    payload.mLightControllers.push_back(settings);
//    LightSettingsMessage message(payload);
//    QHostAddress address;

//    sendToTerminals(message, address);
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
    auto payload = message.payload();

    std::unique_lock<std::mutex> lock(mDbMutex);

    auto database = DatabaseFactory::createDatabaseConnection("lights");

    for(auto& light: payload.mLightControllers)
    {
        auto dbSettings = database->getLightSetting(light.mId);

        if(light.mIsOnChanged)
        {
            LogicState state = boolToLogicState(light.mIsOn); /*light.mIsOn ? LogicState::ON : LogicState::OFF;*/

            mSystemComponents->mGrandCentral->setInputState(database->getLightGroupingId(light.mId), state);
        }

        if(light.mDimmChanged && (light.mType == 1 || light.mType == 2) && light.mDimm != dbSettings.mDimm)
        {
            qDebug() << "setting dimm: "<<light.mDimm;
            database->setLightsDimm(light.mId, light.mDimm);
        }

        if(light.mColorChanged && (light.mType == 2 && light.mColor != dbSettings.mColor))
        {
            qDebug() << "setting color: "<<light.mColor;
            database->setLightsColor(light.mId, light.mColor);
        }
    }
    database.reset();

    lock.unlock();
//    sendToTerminals(message, fromAddr);

    auto header = message.getHeader();

    if(header.mExpectReply)
    {
        ReplyPayload replyPayload(Status::OK);
        ReplyMessage replyMessage(replyPayload);

        qDebug() << "sending reply: "<<replyMessage.toString() <<" to: "<<fromAddr.toString() <<":"<<header.mReplyPort;
        mSystemComponents->mSender->send(fromAddr, header.mReplyPort, replyMessage.toData());
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
    qDebug() << "sending response: "<<respMessage.toString() <<" to: "<<fromAddr.toString() <<":"<<header.mReplyPort;

    mSystemComponents->mSender->send(fromAddr, header.mReplyPort, respMessage.toData());
    qDebug() << "sent";

    mSystemComponents->mGrandCentral->reprovisionOutputValues();
}
