#pragma once

#include <QtSql>
#include <../common/subsystems/heating/HeatingRetrieveStatistics.hpp>


class SensorDatabase
{
public:
    SensorDatabase(const QString& moduleName = "");
    ~SensorDatabase();
    SensorDatabase(const SensorDatabase&) = delete;

    void addSensorTableIfNoExist(const QString& serialNumber);
    void insertRecord(const QString& serialNumber, const QString& timestamp, qint16 value);
    void deleteRecordsByDate(const QString& serialNumber, const QString& date);
    std::vector<TemperatureSensorDataEntry> getRecords(const QString& serialNumber, quint16 limit, quint16 offset = 0);

private:
    QSqlQuery executeSqlQuery(const QString& query);

    static std::recursive_mutex mMutex;
    QString mModuleName;
    QSqlDatabase mDatabase;
};

class SensorDatabaseFactory
{
public:
    static std::unique_ptr<SensorDatabase> createDatabaseConnection(const QString& moduleName = "")
    {
        auto database = std::unique_ptr<SensorDatabase>(new SensorDatabase(moduleName));
        return database;
    }
};
