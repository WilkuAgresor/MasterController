#include "HeatingpAppComponents.hpp"
#include <../common/messages/replyMessage.hpp>

#include <components.hpp>

HeatingAppComponents::HeatingAppComponents(Components *components)
    : mSystemComponents(components)
{

}

HeatSettingsPayload HeatingAppComponents::getCurrentHeatStatus()
{
    HeatSettingsPayload settingsPayload;



//    auto setData = mSensorDataBank.getSensorData();



//    HeatStatusPayload status;

//    auto sensorData = mSensorDataBank.getSensorData();

//    for(auto& sensor: sensorData)
//    {
//        HeatZoneSetting sensorRepresentation;
//        sensorRepresentation.mRoomId = sensor.id;
//        sensorRepresentation.mTemperature = sensor.value;
//        sensorRepresentation.mIsHeatOn = false;
//    }
//    status.mMasterOn = true;
//    status.mCurrentProfile = HeatingProfileType::NIGHT;


    return settingsPayload;
}

void HeatingAppComponents::reprovisionTerminalData(QHostAddress terminalAddr)
{
    HeatSettingsPayload settingsPayload;

    auto database = DatabaseFactory::createDatabaseConnection("heating");

    std::vector<HeatZoneSetting> zoneSettings;

    auto zoneNames = database->getHeatingZoneNames();
    for(const auto& zoneName: zoneNames)
    {
        zoneSettings.emplace_back(database->getHeatZoneSettings(1, zoneName));
    }

    settingsPayload.mMasterOn = database->getHeatMasterOn();
    settingsPayload.mZoneSettings = zoneSettings;
    settingsPayload.mProfiles = database->getHeatProfiles();

    HeatSettingsMessage message(settingsPayload);
    qDebug() <<"heat reprovision settings: " << settingsPayload.toString();


    mSystemComponents->mSender->send(terminalAddr, TERMINAL_LISTEN_PORT, message.toData());
}

void HeatingAppComponents::handleMessage(const Message &message, QHostAddress fromAddr, int fromPort)
{
    auto header = message.getHeader();

    switch(header.getType())
    {
    case MessageType::HEAT_SETTINGS_UPDATE:
        handleSettingsUpdate(static_cast<const HeatSettingsMessage&>(message), fromAddr, fromPort);
        break;

    case MessageType::HEAT_SETTINGS_RETRIEVE:
        handleSettingsRetrieve(static_cast<const HeatRetrieveMessage&>(message), fromAddr);
        break;

        default:
        qDebug() << "received unknown HEAT message";
    }
}

void HeatingAppComponents::handleSettingsUpdate(const HeatSettingsMessage &message, QHostAddress fromAddr, int /*fromPort*/)
{
    qDebug() << "handle settings update";
    auto payload = message.payload();
    auto database = DatabaseFactory::createDatabaseConnection("heating");

    if(payload.mMasterOn != database->getHeatMasterOn())
    {
        database->setHeatMasterOn(payload.mMasterOn);
        qDebug() << "setting master on/off";
    }

    auto profileId = payload.mProfiles.at(0).mId;

    for(auto& zoneSetting: database->getHeatZoneSettings(profileId))
    {
        for(auto& pendingChange: payload.mZoneSettings)
        {
            if(pendingChange.mZoneId == zoneSetting.mZoneId)
            {
                if(pendingChange.mIsOn != zoneSetting.mIsOn)
                {
                    database->setHeatZoneIsOn(pendingChange.mIsOn,
                                              database->getHeatZoneId(pendingChange.mZoneId),
                                              profileId);

                    qDebug() << "setting changed zone on/off";
                }
                if(pendingChange.mSetTemperature != zoneSetting.mSetTemperature)
                {
                    database->setHeatZoneTemperature(pendingChange.mSetTemperature,
                                                     database->getHeatZoneId(pendingChange.mZoneId),
                                                     profileId);
                    qDebug() << "setting changed zone temperature";
                }
                break;
            }
        }
    }

    auto header = message.getHeader();
    if(header.mExpectReply)
    {
        ReplyPayload replyPayload(Status::OK);
        ReplyMessage replyMessage(replyPayload);

        qDebug() << "sending response: "<<replyMessage.toString() <<" to: "<<fromAddr.toString() <<":"<<header.mReplyPort;

        mSystemComponents->mSender->send(fromAddr, header.mReplyPort, replyMessage.toData());
        qDebug() << "sent";

    }
}

void HeatingAppComponents::handleSettingsRetrieve(const HeatRetrieveMessage &message, QHostAddress fromAddr)
{
    auto payload = message.payload();
    int profileId = payload.mProfileId;
    qDebug() << "handle settings retrieve for profile: "<<profileId;

    auto database = DatabaseFactory::createDatabaseConnection("heating");

    HeatSettingsPayload respPayload;
    respPayload.mMasterOn = database->getHeatMasterOn();
    respPayload.mZoneSettings = database->getHeatZoneSettings(profileId);

    HeatSettingsMessage respMessage(respPayload);

    auto header = message.getHeader();
    qDebug() << "sending response: "<<respMessage.toString() <<" to: "<<fromAddr.toString() <<":"<<header.mReplyPort;

    mSystemComponents->mSender->send(fromAddr, header.mReplyPort, respMessage.toData());
    qDebug() << "sent";
}

