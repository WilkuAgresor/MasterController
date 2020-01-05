#include "SensorData.hpp"
#include <../common/subsystems/heating/HeatingDictionary.hpp>

TemperatureSensorDataBank::TemperatureSensorDataBank()
{
    for(const auto& id: sHeatingIds)
    {
        mSensorData.emplace_back(TemperatureSensorEntry(id));
    }
}

void TemperatureSensorDataBank::pollSensors()
{
    std::lock_guard<std::mutex> _lock(mMutex);

    for(auto& sensorEntry: mSensorData)
    {
        sensorEntry.value = getCurrentTemperature(sensorEntry.id);
    }
}

quint16 TemperatureSensorDataBank::getCurrentTemperature(const QString& /*hwId*/)
{
    return 26;
}

