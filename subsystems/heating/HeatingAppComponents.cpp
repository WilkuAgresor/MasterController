#include "HeatingAppComponents.hpp"
#include <../common/messages/replyMessage.hpp>
#include "SensorDatabase.hpp"
#include <components.hpp>
#include <QDateTime>
#include <LeonardoIpExecutor/LeoMessage.hpp>
#include <LeonardoIpExecutor/OpenthermMessages.hpp>
#include <ranges>


HeatingAppComponents::HeatingAppComponents(QObject *parent, Components *components)
    : QObject(parent)
    , mSystemComponents(components)
{
    QObject::connect(mSystemComponents, SIGNAL(hardwareReprovisionNotif(ControllerInfo)), this, SLOT(handleHardwareReprovisionNotif(ControllerInfo)));
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

    std::unique_lock<std::mutex> lock(mDbMutex);
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

    database.reset();
    lock.unlock();

    mSystemComponents->mSender->send(terminalAddr, TERMINAL_LISTEN_PORT, message.toData());

    lock.lock();
    database = DatabaseFactory::createDatabaseConnection("heating");
    auto boilerSettings = database->getBoilerSettings();

    if(!boilerSettings)
    {
        qDebug() << "no boiler settings 1";
    }

    database.reset();
    lock.unlock();

    if(boilerSettings)
    {
        BoilerSettingsMessage boilerMessage(*boilerSettings);
        mSystemComponents->mSender->send(terminalAddr, TERMINAL_LISTEN_PORT, boilerMessage.toData());
    }
}

void HeatingAppComponents::updateTerminalCurrentTemperatures(QHostAddress terminalAddr)
{
    HeatSettingsPayload settingsPayload;

    {
        std::unique_lock<std::mutex> lock(mDbMutex);
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
    }


    HeatSettingsMessage message(settingsPayload);
//    qDebug() <<"temperature update: " << settingsPayload.toString();

    mSystemComponents->mSender->send(terminalAddr, TERMINAL_LISTEN_PORT, message.toData());
}

void HeatingAppComponents::updateTerminalBoilerSettings(QHostAddress terminalAddr)
{
    std::unique_lock<std::mutex> lock(mDbMutex);

    auto database = DatabaseFactory::createDatabaseConnection("heating");

    auto boilerSettings = database->getBoilerSettings();

    if(!boilerSettings)
    {
        qDebug() << "no boiler settings 2";
    }

    database.reset();
    lock.unlock();

    if(boilerSettings)
    {
        BoilerSettingsMessage message(*boilerSettings);
        mSystemComponents->mSender->send(terminalAddr, TERMINAL_LISTEN_PORT, message.toData());
    }
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

    case MessageType::BOILER_SETTINGS:
        handleBoilerSettingsUpdate(static_cast<const BoilerSettingsMessage&>(message), fromAddr);
        break;

        default:
        qDebug() << "received unknown HEAT message";
    }
}

void HeatingAppComponents::handleSettingsUpdate(const HeatSettingsMessage &message, QHostAddress fromAddr)
{
    qDebug() << "handle settings update";
    auto payload = message.payload();

    {
        std::unique_lock<std::mutex> lock(mDbMutex);
        auto database = DatabaseFactory::createDatabaseConnection("heating");

        if(payload.mMasterOn != database->getHeatMasterOn())
        {
            database->setHeatMasterOn(payload.mMasterOn);
            qDebug() << "setting master on/off";
        }

        auto profileId = payload.mProfiles.at(0).mId;

        auto heatZoneSettings = database->getHeatZoneSettings(profileId);

        for(const auto& pendingChange: payload.mZoneSettings)
        {
            if( auto currentSettingOpt = std::ranges::find_if(heatZoneSettings, [&pendingChange](const auto& zoneSetting) {return pendingChange.mZoneId == zoneSetting.mZoneId;});
                currentSettingOpt != heatZoneSettings.end())
            {
                if(pendingChange.mIsOn != currentSettingOpt->mIsOn)
                {
                    database->setHeatZoneIsOn(pendingChange.mIsOn,
                                              pendingChange.mZoneId,
                                              profileId);

                    qDebug() << "setting changed zone on/off";
                }
                if(pendingChange.mSetTemperature != currentSettingOpt->mSetTemperature)
                {
                    database->setHeatZoneTemperature(pendingChange.mSetTemperature,
                                                     pendingChange.mZoneId,
                                                     profileId);
                    qDebug() << "setting changed zone temperature";
                }
            }
        }
    }

    auto header = message.getHeader();
    if(header.mExpectReply)
    {
        ReplyPayload replyPayload(Status::OK);
        ReplyMessage replyMessage(replyPayload);

        mSystemComponents->mSender->send(fromAddr, header.mReplyPort, replyMessage.toData());
    }
}

void HeatingAppComponents::handleSettingsRetrieve(const HeatRetrieveMessage &message, QHostAddress fromAddr)
{
    auto payload = message.payload();
    int profileId = payload.mProfileId;
    qDebug() << "handle settings retrieve for profile: "<<profileId;
    HeatSettingsPayload respPayload;

    {
        std::unique_lock<std::mutex> lock(mDbMutex);
        auto database = DatabaseFactory::createDatabaseConnection("heating");

        respPayload.mMasterOn = database->getHeatMasterOn();
        respPayload.mZoneSettings = database->getHeatZoneSettings(profileId);
    }

    HeatSettingsMessage respMessage(respPayload);

    auto header = message.getHeader();

    mSystemComponents->mSender->send(fromAddr, header.mReplyPort, respMessage.toData());
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

        auto zoneIt = std::find_if(mHeatingHardware.mHeatingZones.begin(), mHeatingHardware.mHeatingZones.end(),
                                   [zoneId](const auto& zone) { return zone.mId == zoneId; });

        if (zoneIt != mHeatingHardware.mHeatingZones.end())
        {
            auto sensorIt = std::find_if(zoneIt->mSensors.begin(), zoneIt->mSensors.end(),
                                         [&payload](const auto& sensor) { return sensor.mType == payload.mSensorType; });

            if (sensorIt != zoneIt->mSensors.end())
            {
                serialNumber = sensorIt->mSerialNumber;
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
        startTime = QDateTime(finishTime.date().addDays(-1), finishTime.time());
        break;
    case TemperatureStatisticsTimeScope::TWO_DAYS:
        startTime = QDateTime(finishTime.date().addDays(-2), finishTime.time());
        break;
    case TemperatureStatisticsTimeScope::ONE_WEEK:
        startTime = QDateTime(finishTime.date().addDays(-14), finishTime.time());
        break;
    }

    quint16 pageSize = 150;
    quint32 offset = 0;

    auto db = SensorDatabaseFactory::createDatabaseConnection("StatRetrieve");

    std::vector<TemperatureSensorDataEntry> page;
    do
    {
        page = db->getRecords(serialNumber, pageSize, offset);
        offset += pageSize;

        for(const auto& record: page)
        {
            const auto date = QDateTime::fromString(record.mTimestamp, Qt::ISODate);
            if(date > startTime && date < finishTime)
            {
                entryList.mEntries.push_back(record);
            }
        }
        qDebug() << "page processed";
    }
    while (!page.empty() && page.size() >= pageSize);

    HeatRetrieveStatisticsResponse respMessage(entryList);

    qWarning() << "statistics processing finished, entries qualified: "<<entryList.mEntries.size() <<" message size: "<<respMessage.toData().size() <<" bytes";

    mSystemComponents->mSender->send(fromAddr, header.mReplyPort, respMessage.toData());
    qWarning() << "statistics message sent";
    return;
}

void HeatingAppComponents::handleBoilerSettingsUpdate(const BoilerSettingsMessage &message, QHostAddress /*fromAddr*/)
{
    auto payload = message.payload();
    bool changed = false;

    {
        std::unique_lock<std::mutex> lock(mDbMutex);
        auto database = DatabaseFactory::createDatabaseConnection("heating");

        if(payload.mCentralHeatingTempSetChanged)
        {
            database->setBoilerSettingByName(JSON_CENTRAL_HEATING_TEMP_SET, payload.getCentralHeatingTempSet());
            changed = true;
            updateBoilerHardware(JSON_CENTRAL_HEATING_TEMP_SET, payload.getCentralHeatingTempSet());

        }
        if(payload.mMaxCentralHeatingTempSetChanged)
        {
            database->setBoilerSettingByName(JSON_MAX_CENTRAL_HEATING_TEMP_SET, payload.getMaxCentralHeatingTempSet());
            changed = true;
            updateBoilerHardware(JSON_MAX_CENTRAL_HEATING_TEMP_SET, payload.getMaxCentralHeatingTempSet());
        }
        if(payload.mEnableCentralHeatingChanged)
        {
            database->setBoilerSettingByName(JSON_ENABLE_CENTRAL_HEATING, payload.getEnableCentralHeating());
            changed = true;
            updateBoilerHardware(JSON_ENABLE_CENTRAL_HEATING, payload.getEnableCentralHeating() ? 1 : 0);
        }
        if(payload.mEnableHotWaterChanged)
        {
            database->setBoilerSettingByName(JSON_ENABLE_HOT_WATER, payload.getEnableHotWater());
            changed = true;
            updateBoilerHardware(JSON_ENABLE_HOT_WATER, payload.getEnableHotWater() ? 1 : 0);
        }
        if(payload.mHotWaterTempSetChanged)
        {
            database->setBoilerSettingByName(JSON_HOT_WATER_TEMP_SET, payload.getHotWaterTempSet());
            changed = true;
            updateBoilerHardware(JSON_HOT_WATER_TEMP_SET, payload.getHotWaterTempSet());
        }
    }
    if(changed)
    {
        for(const auto& terminal: mSystemComponents->mControllers | std::views::filter([](const auto& controller){return controller.type == ControllerInfo::Type::TERMINALv1;}))
        {
            auto address = QHostAddress(terminal.ipAddr);
            updateTerminalBoilerSettings(address);
        }
    }

}

void HeatingAppComponents::updateBoilerHardware(const QString &name, int value)
{
    auto boilerControllerIt = std::ranges::find_if(mSystemComponents->mControllers, [this](const auto& controller){return isBoilerControllerById(controller.name);});

    if(boilerControllerIt != mSystemComponents->mControllers.end())
    {
       OpenthermSetMessage setMessage(name, value);

       auto sendResult = mSystemComponents->mSender->sendRaw(boilerControllerIt->getIpAddress(), boilerControllerIt->port, setMessage.serialize());

       if(!sendResult)
       {
           qDebug() << "controller unreachable for boiler settings reprovision 2";
       }
    }
}

void HeatingAppComponents::handleBoilerStateUpdate(const BoilerSettingsPayload& boilerSettings)
{
    std::unique_lock<std::mutex> lock(mDbMutex);

    auto database = DatabaseFactory::createDatabaseConnection("heating");

    //TODO: Boiler status statictics

    if(boilerSettings.mConnectionStatus == 0)
    {
        database->setBoilerSettingByName(JSON_IS_CENTRAL_HEATING_ON, boilerSettings.mIsCentralHeatingOn ? 1 : 0);
        database->setBoilerSettingByName(JSON_IS_HOT_WATER_ON, boilerSettings.mIsHotWaterOn ? 1 : 0);
        database->setBoilerSettingByName(JSON_IS_FLAME_ON, boilerSettings.mIsFlameOn ? 1 : 0);
        database->setBoilerSettingByName(JSON_CENTRAL_HEATING_TEMP, boilerSettings.mCentralHeatingTemp);
        database->setBoilerSettingByName(JSON_MAX_CENTRAL_HEATING_TEMP, boilerSettings.mCentralHeatingTemp);
        database->setBoilerSettingByName(JSON_HOT_WATER_TEMP, boilerSettings.mHotWaterTemp);
        database->setBoilerSettingByName(JSON_CONNECTION_STATUS, boilerSettings.mConnectionStatus);
        database->setBoilerSettingByName(JSON_RETURN_TEMP, boilerSettings.mReturnTemp);
        database->setBoilerSettingByName(JSON_PRESSURE, boilerSettings.mPressure);
    }
    else
    {
        database->setBoilerSettingByName(JSON_IS_CENTRAL_HEATING_ON, 0);
        database->setBoilerSettingByName(JSON_IS_HOT_WATER_ON, 0);
        database->setBoilerSettingByName(JSON_IS_FLAME_ON, 0);
        database->setBoilerSettingByName(JSON_CENTRAL_HEATING_TEMP, 0);
        database->setBoilerSettingByName(JSON_MAX_CENTRAL_HEATING_TEMP, 0);
        database->setBoilerSettingByName(JSON_HOT_WATER_TEMP, 0);
        database->setBoilerSettingByName(JSON_CONNECTION_STATUS, boilerSettings.mConnectionStatus);
        database->setBoilerSettingByName(JSON_RETURN_TEMP, 0);
        database->setBoilerSettingByName(JSON_PRESSURE, 0);
    }
}

bool HeatingAppComponents::isBoilerControllerById(const QString &controllerId)
{
    return controllerId == "boilerController";
}

void HeatingAppComponents::setupEmptyBoilerSettings()
{
    std::unique_lock<std::mutex> lock(mDbMutex);
    auto database = DatabaseFactory::createDatabaseConnection("heating");
    //create empty table
    database->createBoilerSettingsTable();
    //hot water always on
    database->setBoilerSettingByName(JSON_ENABLE_HOT_WATER, 1);
    //max central heating temp to 35
    database->setBoilerSettingByName(JSON_MAX_CENTRAL_HEATING_TEMP_SET,35);
}

void HeatingAppComponents::handleHardwareReprovisionNotif(ControllerInfo controller)
{
    if(controller.type == ControllerInfo::Type::ARD_LEO && isBoilerControllerById(controller.name))
    {
        qDebug() << "reprovision boiler hardware settings";
        std::optional<BoilerSettingsPayload> boilerSettings;

        {
            std::unique_lock<std::mutex> lock(mDbMutex);
            auto database = DatabaseFactory::createDatabaseConnection("heating");

            boilerSettings = database->getBoilerSettings();
        }

        if(!boilerSettings)
        {
            qDebug() << "Failed to retrieve boilerSettings";
            return;
        }

        auto settableNameValuePairs = boilerSettings->getSettableNameValuePairs();

        for(const auto& [name,value]: settableNameValuePairs)
        {
            OpenthermSetMessage setMessage(name, value);

            qDebug() << "message: "<<setMessage.serialize();

            auto sendResult = mSystemComponents->mSender->sendRaw(controller.getIpAddress(), controller.port, setMessage.serialize());

            if(!sendResult)
            {
                qDebug() << "controller unreachable for boiler settings reprovision 2";
            }
        }
    }
}
