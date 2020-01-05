#include "HeatingpAppComponents.hpp"

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

    for(auto& zone: zoneSettings)
    {
        qDebug() << "zone setting: "<<zone.toString().toUtf8().constData();
    }
    settingsPayload.mCurrentProfile = HeatProfile(1, database->getHeatProfileName(1));
    settingsPayload.mMasterOn = database->getHeatMasterOn();
    settingsPayload.mZoneSettings = zoneSettings;

    HeatSettingsMessage message(settingsPayload);

    qDebug() <<"heat reprovision settings: " << settingsPayload.toString();


    mSystemComponents->mSender->send(terminalAddr, TERMINAL_LISTEN_PORT, message.toData());

//    database->get


//    for(const auto& controller: mSystemComponents->mControllers)
//    {
//        if( controller.type == ControllerInfo::Type::TERMINALv1)
//        {

//            QHostAddress address(controller.ipAddr);
//            mSystemComponents->mSender->send(address, TERMINAL_LISTEN_PORT, message.toData());

////            QHostAddress address(controller.ipAddr);
////            mComponents.mSender->send(address,TERMINAL_LISTEN_PORT, message.toData());
//        }
//    }
}
