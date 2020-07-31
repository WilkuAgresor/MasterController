#pragma once

#include <QString>
#include <mutex>
#include <map>
#include <../common/subsystems/heating/HeatingDictionary.hpp>
#include <QDebug>

const QString TEMPERATURE_SENSOR_PATH = "/mnt/1wire/";
const QString TEMPERATURE_SENSOR_PRECISION = "/temperature11";
const QString HEATING_HW_DB_CONNECTION = "HeatingHw";

struct TemperatureSensor
{
    TemperatureSensor(quint16 id, const QString& serialNumber, TemperatureSensorType type):
        mId(id), mSerialNumber(serialNumber), mType(type)
    {
        qDebug() <<" sensor type: "<< QString::number(static_cast<int>(mType));

    }

    void refreshTemperatureReading();

    quint16 mId;
    QString mSerialNumber;
    qint16 mCurrentTemperature = TEMPERATURE_INVALID;
    TemperatureSensorType mType;
    quint8 mRetryCounter = 0;

private:
    QString getSensorPath();
};

struct HeatingZone
{
    HeatingZone(quint16 zoneId);

    void addTemperatureSensor(quint16 sensorId, const QString& serialNumber, TemperatureSensorType type);

    int16_t getCurrentTemperature();
    void refreshTemperatureReading();

    quint16 mId;
    bool mIsOn = false;
    quint16 mSubsystemId;

    std::vector<TemperatureSensor> mSensors;
};

struct HeatingSubsystem
{
    quint16 mId;
    bool mIsOn = false;
};

struct HeatingHardware
{
    HeatingHardware();

    void setMasterOn(bool isOn);
    bool getMasterOn();

    void addHeatingZone(quint16 id, quint16 subsystemId);
    void addTemperatureSensor(quint16 id, quint16 zoneId, const QString& serialNumber, TemperatureSensorType type);

    void setZoneIsOn(quint16 zoneId, bool isOn);
    void setSubsystemIsOn(quint16 subsystemId, bool isOn);

    qint16 getZoneCurrentTemperature(quint16 zoneId);
    bool getZoneIsOn(quint16 zoneId);
    bool getSubsystemIsOn(quint16 subsystemId);

    void refreshTemperatureReading();

    std::vector<HeatingSubsystem> mHeatingSubsystems;
    std::vector<HeatingZone> mHeatingZones;
    bool masterOn = false;
};

class TemperatureSensorDataBank
{
public:
    TemperatureSensorDataBank();

    void pollSensors();

private:
    quint16 getCurrentTemperature(const QString &hwId);
    std::mutex mMutex;
};


/*
 * CREATE TABLE "28.0D41070A0001" (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT,
    value INTEGER
);
 */



