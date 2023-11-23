#include "HeatingAppComponents.hpp"
#include <../common/messages/replyMessage.hpp>
#include "SensorDatabase.hpp"
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

    auto zoneIds = database->getHeatingZoneIds();
    for(const auto& zoneId: zoneIds)
    {
        auto zoneSetting = database->getHeatZoneSettings(1, zoneId);
        zoneSetting.mCurrentTemp = mHeatingHardware.getZoneCurrentTemperature(zoneSetting.mZoneId);

        zoneSettings.push_back(zoneSetting);
    }

    settingsPayload.mMasterOn = database->getHeatMasterOn();
    settingsPayload.mZoneSettings = zoneSettings;
    settingsPayload.mProfiles = database->getHeatProfiles();

    HeatSettingsMessage message(settingsPayload);
    qDebug() <<"heat reprovision settings: " << settingsPayload.toString();


    mSystemComponents->mSender->send(terminalAddr, TERMINAL_LISTEN_PORT, message.toData());
}

void HeatingAppComponents::updateTerminalCurrentTemperatures(QHostAddress terminalAddr)
{
    HeatSettingsPayload settingsPayload;

    auto database = DatabaseFactory::createDatabaseConnection("heating");

    std::vector<HeatZoneSetting> zoneSettings;

    auto zoneIds = database->getHeatingZoneIds();
    for(const auto& zoneId: zoneIds)
    {
        auto zoneSetting = database->getHeatZoneSettings(1, zoneId);

        zoneSetting.mCurrentTempChanged = true;
        zoneSetting.mCurrentTemp = mHeatingHardware.getZoneCurrentTemperature(zoneSetting.mZoneId);

        zoneSettings.push_back(zoneSetting);
    }

    settingsPayload.mMasterOn = database->getHeatMasterOn();
    settingsPayload.mZoneSettings = zoneSettings;

    HeatSettingsMessage message(settingsPayload);
//    qDebug() <<"temperature update: " << settingsPayload.toString();

    mSystemComponents->mSender->send(terminalAddr, TERMINAL_LISTEN_PORT, message.toData());
}

void HeatingAppComponents::handleMessage(const Message &message, QHostAddress fromAddr)
{
    auto header = message.getHeader();

    switch(header.getType())
    {
    case MessageType::HEAT_SETTINGS_UPDATE:
        handleSettingsUpdate(static_cast<const HeatSettingsMessage&>(message), fromAddr);
        break;

    case MessageType::HEAT_SETTINGS_RETRIEVE:
        handleSettingsRetrieve(static_cast<const HeatRetrieveMessage&>(message), fromAddr);
        break;
    case MessageType::HEAT_STATISTICS_RETRIEVE:
        handleStatisticsRetrieve(static_cast<const HeatRetrieveStatisticsMessage&>(message), fromAddr);
        break;

        default:
        qDebug() << "received unknown HEAT message";
    }
}

void HeatingAppComponents::handleSettingsUpdate(const HeatSettingsMessage &message, QHostAddress fromAddr)
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
                                              pendingChange.mZoneId,
                                              profileId);

                    qDebug() << "setting changed zone on/off";
                }
                if(pendingChange.mSetTemperature != zoneSetting.mSetTemperature)
                {
                    database->setHeatZoneTemperature(pendingChange.mSetTemperature,
                                                     pendingChange.mZoneId,
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

void HeatingAppComponents::handleStatisticsRetrieve(const HeatRetrieveStatisticsMessage &message, QHostAddress fromAddr)
{
    auto payload = message.payload();
    qDebug() << "handle statistics retrieve for zone: "<<payload.mZoneName;
    auto header = message.getHeader();

    QString serialNumber;
    TemperatureSensorEntryList entryList;

    {
        auto db = DatabaseFactory::createDatabaseConnection("HeatStatRetrieve");
        auto zoneId = db->getHeatZoneId(payload.mZoneName);
        for(auto& zone: mHeatingHardware.mHeatingZones)
        {
            if(zone.mId == zoneId)
            {
                for(auto sensor: zone.mSensors)
                {
                    if(sensor.mType == payload.mSensorType)
                    {
                        serialNumber = sensor.mSerialNumber;
                        break;
                    }
                }
                break;
            }
        }
    }

    qDebug() << "sensor: "<< serialNumber;

    if(serialNumber.isEmpty())
    {
        qDebug() << "No valid sensor exist for this Heating Zone";
        HeatRetrieveStatisticsResponse respMessage(entryList);
        mSystemComponents->mSender->send(fromAddr, header.mReplyPort, respMessage.toData());
        return;
    }

    auto finishTime = QDateTime::currentDateTime();
    QDateTime startTime;

    switch(payload.mTimeScope)
    {
    case TemperatureStatisticsTimeScope::DAY:
        startTime = QDateTime(finishTime.date().addDays(-1));
        break;
    case TemperatureStatisticsTimeScope::TWO_DAYS:
        startTime = QDateTime(finishTime.date().addDays(-2));
        break;
    case TemperatureStatisticsTimeScope::ONE_WEEK:
        startTime = QDateTime(finishTime.date().addDays(-14));
        break;
    }

    quint16 pageSize = 150;
    quint32 offset = 0;

    auto db = SensorDatabaseFactory::createDatabaseConnection("StatRetrieve");

    std::vector<TemperatureSensorDataEntry> page;
    do
    {
        page.clear();
        page = db->getRecords(serialNumber, pageSize, offset);
        offset += pageSize;
        for(auto& record: page)
        {
            auto date = QDateTime::fromString(record.mTimestamp, Qt::ISODate);
            if(date > startTime && date < finishTime)
            {
                entryList.mEntries.push_back(record);
            }
        }
        qDebug() << "page finished";
    }
    while (page.size() >= pageSize);

    HeatRetrieveStatisticsResponse respMessage(entryList);

    qWarning() << "statistics processing finished, entries qualified: "<<entryList.mEntries.size() <<" message size: "<<respMessage.toData().size() <<" bytes";

    mSystemComponents->mSender->send(fromAddr, header.mReplyPort, respMessage.toData());
    qWarning() << "statistics message sent";
    return;
}
