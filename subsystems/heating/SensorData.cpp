#include "SensorData.hpp"
#include <../common/subsystems/heating/HeatingDictionary.hpp>
#include <numeric>
#include <QFile>
#include <database/database.hpp>
#include <QtConcurrent/QtConcurrent>
#include <QDateTime>
#include <ranges>

#include "SensorDatabase.hpp"

TemperatureSensorDataBank::TemperatureSensorDataBank()
{

}

void TemperatureSensorDataBank::pollSensors()
{
    std::lock_guard<std::mutex> _lock(mMutex);
}

quint16 TemperatureSensorDataBank::getCurrentTemperature(const QString& /*hwId*/)
{
    return 26;
}


HeatingHardware::HeatingHardware()
{
    auto db = DatabaseFactory::createDatabaseConnection(HEATING_HW_DB_CONNECTION);
    mHeatingZones = db->getHeatingZonesHardware();

    for(auto& zone: mHeatingZones)
    {
        qDebug() <<"Heat zone: "<<QString::number(zone.mId);

        zone.mSensors = db->getTemperatureSensorsHardware(zone.mId);

        auto db = SensorDatabaseFactory::createDatabaseConnection("Hardware");

        for(auto& sensor: zone.mSensors)
        {
            db->addSensorTableIfNoExist(sensor.mSerialNumber);
        }
    }
}

void HeatingHardware::setZoneIsOn(quint16 zoneId, bool isOn)
{
    if (auto it = std::ranges::find_if(mHeatingZones, [zoneId](const auto& zone) { return zone.mId == zoneId; });
        it != mHeatingZones.end())
    {
        it->mIsOn = isOn;
    }
}

void HeatingHardware::setSubsystemIsOn(quint16 subsystemId, bool isOn)
{
    if (auto it = std::ranges::find_if(mHeatingSubsystems, [subsystemId](const auto& subsystem) { return subsystem.mId == subsystemId; });
        it != mHeatingSubsystems.end())
    {
        it->mIsOn = isOn;
    }
}

qint16 HeatingHardware::getZoneCurrentTemperature(quint16 zoneId)
{
    if (auto it = std::ranges::find_if(mHeatingZones, [zoneId](const auto& zone) { return zone.mId == zoneId; });
        it != mHeatingZones.end())
    {
        return it->getCurrentTemperature();
    }
    return TEMPERATURE_INVALID;
}

bool HeatingHardware::getZoneIsOn(quint16 zoneId)
{
    if (auto it = std::ranges::find_if(mHeatingZones, [zoneId](const auto& zone) { return zone.mId == zoneId; });
        it != mHeatingZones.end())
    {
        return getSubsystemIsOn(it->mSubsystemId) ? it->mIsOn : false;
    }
    return false;
}

bool HeatingHardware::getSubsystemIsOn(quint16 subsystemId)
{
    if (auto it = std::ranges::find_if(mHeatingSubsystems, [subsystemId](const auto& subsystem) { return subsystem.mId == subsystemId; });
        it != mHeatingSubsystems.end())
    {
        return it->mIsOn;
    }
    return false;
}

void HeatingHardware::refreshTemperatureReading()
{
    std::ranges::for_each(mHeatingZones, [](auto& zone) { zone.refreshTemperatureReading(); });
}

HeatingZone::HeatingZone(quint16 zoneId):
    mId(zoneId)
{
}

void HeatingZone::addTemperatureSensor(quint16 sensorId, const QString &serialNumber, TemperatureSensorType type)
{
    mSensors.emplace_back(TemperatureSensor(sensorId, serialNumber, type));
}

int16_t HeatingZone::getCurrentTemperature()
{
    std::vector<int16_t> relevantTemperatures;
    for(auto sensor: mSensors | std::views::filter([](const auto& x){ return x.mType == TemperatureSensorType::AIR;}))
    {
        relevantTemperatures.push_back(sensor.mCurrentTemperature);
    }

    if(relevantTemperatures.size() == 1)
    {
        return relevantTemperatures[0];
    }
    else if(relevantTemperatures.size() > 1)
    {   //zwykła średnia ze wszystkich czujników teperatury powietrza
        return accumulate(relevantTemperatures.begin(), relevantTemperatures.end(), 0) / relevantTemperatures.size();
    }
    return TEMPERATURE_INVALID;
}

void HeatingZone::refreshTemperatureReading()
{
    std::ranges::for_each(mSensors, [](auto& sensor) { sensor.refreshTemperatureReading(); });
}

void TemperatureSensor::refreshTemperatureReading()
{
    bool ok = true;

    QFile file(getSensorPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open Temperature Sensor file";
        ok = false;
    }

    if(ok)
    {
        auto tempString = QString(file.readLine());
        qfloat16 realValue = tempString.toFloat(&ok);

        if(ok)
        {
            mRetryCounter = 0;
            realValue  = realValue * 100;
            auto newValue = static_cast<qint16>(realValue);
            if(newValue != mCurrentTemperature)
            {
                mCurrentTemperature = newValue;

                auto db = SensorDatabaseFactory::createDatabaseConnection("SensorRefresh");
                db->insertRecord(mSerialNumber, QDateTime::currentDateTime().toString(Qt::ISODate), mCurrentTemperature);
            }
        }
    }

    if(!ok)
    {
        qDebug() << "reading invalid";

        if(mRetryCounter >= 5)
        {
            mCurrentTemperature = TEMPERATURE_INVALID;
        }
        else
        {
            mRetryCounter++;
        }
    }
}

QString TemperatureSensor::getSensorPath()
{
    return TEMPERATURE_SENSOR_PATH + mSerialNumber + TEMPERATURE_SENSOR_PRECISION;
}
