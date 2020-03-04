#include "LightsAppComponents.hpp"
#include <components.hpp>

#include <../common/messages/replyMessage.hpp>

LightsAppComponents::LightsAppComponents(Components *components)
    : mSystemComponents(components)
{
}

void LightsAppComponents::reprovisionTerminalData(QHostAddress terminalAddr)
{
    LightSettingsPayload settingsPayload;

    auto database = DatabaseFactory::createDatabaseConnection("lights");

    auto lightSettings = database->getLightSettings();
    settingsPayload.mLightControllers = lightSettings;

    LightSettingsMessage message(settingsPayload);
    qDebug() <<"lights reprovision settings: " << settingsPayload.toString();

    mSystemComponents->mSender->send(terminalAddr, TERMINAL_LISTEN_PORT, message.toData());
}

void LightsAppComponents::handleMessage(const Message &message, QHostAddress fromAddr, int fromPort)
{
    auto header = message.getHeader();

    switch(header.getType())
    {
    case MessageType::LIGHTS_UPDATE:
        handleLightsUpdate(static_cast<const LightSettingsMessage&>(message), fromAddr, fromPort);
        break;

//    case MessageType::HEAT_SETTINGS_RETRIEVE:
//        handleLightsRetrieve(static_cast<const LightsRetrieveMessage&>(message), fromAddr);
//        break;

        default:
        qDebug() << "received unknown Lights message";
    }
}

void LightsAppComponents::handleLightsUpdate(const LightSettingsMessage &message, QHostAddress fromAddr, int /*fromPort*/)
{
    qDebug() << "handle lights update";
    auto payload = message.payload();
    auto database = DatabaseFactory::createDatabaseConnection("lights");

    for(auto& light: payload.mLightControllers)
    {
        auto dbSettings = database->getLightSetting(light.mId);

        if(light.mIsOn != dbSettings.mIsOn)
        {
            database->setLightsIsOn(light.mId, light.mIsOn);
        }

        if((light.mType == 1 || light.mType == 2) && light.mDimm != dbSettings.mDimm)
        {
            database->setLightsDimm(light.mId, light.mDimm);
        }

        if(light.mType == 2 && light.mColor != dbSettings.mColor)
        {
            database->setLightsColor(light.mId, light.mColor);
        }
    }

    auto header = message.getHeader();

    if(header.mExpectReply)
    {
        ReplyPayload replyPayload(Status::OK);
        ReplyMessage replyMessage(replyPayload);

        qDebug() << "sending response: "<<replyMessage.toString() <<" to: "<<fromAddr.toString() <<":"<<header.mReplyPort;
        mSystemComponents->mSender->send(fromAddr, header.mReplyPort, replyMessage.toData());
    }
}

void LightsAppComponents::handleLightsRetrieve(const LightsRetrieveMessage &message, QHostAddress fromAddr)
{
    auto database = DatabaseFactory::createDatabaseConnection("lights");

    LightSettingsPayload respPayload;
    respPayload.mLightControllers = database->getLightSettings();

    LightSettingsMessage respMessage(respPayload);

    auto header = message.getHeader();
    qDebug() << "sending response: "<<respMessage.toString() <<" to: "<<fromAddr.toString() <<":"<<header.mReplyPort;

    mSystemComponents->mSender->send(fromAddr, header.mReplyPort, respMessage.toData());
    qDebug() << "sent";
}

