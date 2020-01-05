#pragma once

#include <QString>
#include <mutex>
#include <map>

struct TemperatureSensorEntry
{
    TemperatureSensorEntry(const QString& ids, quint16 values = 0):
        id(ids), value(values)
    {}

    QString id;
    quint16 value;
};

class TemperatureSensorDataBank
{
public:
    TemperatureSensorDataBank();

    void pollSensors();

    std::vector<TemperatureSensorEntry> getSensorData()
    {
        std::lock_guard<std::mutex> _lock(mMutex);
        return mSensorData;
    }

private:
    quint16 getCurrentTemperature(const QString &hwId);


    std::mutex mMutex;
    std::vector<TemperatureSensorEntry> mSensorData;

    const std::map<QString, QString> mIdMapping {
        { "salon", "id1" },
        { "wiatrolap", "id2" },
        { "gabinet", "id3" },
        { "lazienka1", "id4" },
        { "lazienka2", "id5" },
        { "lazienka3", "id6" },
        { "sypialnia1", "id7" },
        { "sypialnia2", "id8" },
        { "sypialnia3", "id9" },
        { "sypialnia4", "id10" },
        { "garderoba1", "id11" },
        { "garderoba2", "id12" }
    };

};



