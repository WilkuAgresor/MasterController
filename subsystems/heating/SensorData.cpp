#include "SensorData.hpp"
#include <../common/subsystems/heating/HeatingDictionary.hpp>
#include <numeric>
#include <QFile>
#include <database/database.hpp>
#include <QtConcurrent/QtConcurrent>
#include <QDateTime>

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
    for(auto& zone : mHeatingZones)
    {
        if(zone.mId == zoneId)
        {
            zone.mIsOn = isOn;
            return;
        }
    }
}

void HeatingHardware::setSubsystemIsOn(quint16 subsystemId, bool isOn)
{
    for(auto& subsystem : mHeatingSubsystems)
    {
        if(subsystem.mId == subsystemId)
        {
            subsystem.mIsOn = isOn;
            return;
        }
    }
}

qint16 HeatingHardware::getZoneCurrentTemperature(quint16 zoneId)
{
    for(auto& zone : mHeatingZones)
    {
        if(zone.mId == zoneId)
        {
            return zone.getCurrentTemperature();
        }
    }
    return TEMPERATURE_INVALID;
}

bool HeatingHardware::getZoneIsOn(quint16 zoneId)
{
    for(auto& zone : mHeatingZones)
    {
        if(zone.mId == zoneId)
        {
            if(getSubsystemIsOn(zone.mSubsystemId))
            {
               return zone.mIsOn;
            }
            return false;
        }
    }
    return false;
}

bool HeatingHardware::getSubsystemIsOn(quint16 subsystemId)
{
    for(auto& subsystem : mHeatingSubsystems)
    {
        if(subsystem.mId == subsystemId)
        {
            return subsystem.mIsOn;
        }
    }
    return false;
}

void HeatingHardware::refreshTemperatureReading()
{
    for(auto& zone: mHeatingZones)
    {
        zone.refreshTemperatureReading();
    }
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
    for(auto sensor: mSensors)
    {
        if(sensor.mType == TemperatureSensorType::AIR)
        {
            relevantTemperatures.push_back(sensor.mCurrentTemperature);
        }
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
    for(auto& sensor: mSensors)
    {
        sensor.refreshTemperatureReading();
    }
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
