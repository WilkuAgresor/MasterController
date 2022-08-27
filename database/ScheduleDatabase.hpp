#pragma once

#include <QtSql>
#include <../common/subsystems/schedule/ScheduleEventTypes.hpp>


class ScheduleDatabase
{
public:
    ScheduleDatabase(const QString& moduleName = "");
    ~ScheduleDatabase();
    ScheduleDatabase(const ScheduleDatabase&) = delete;

    void insertEvent(int evtType, const QString& body, int intervalProfile, const QString& triggerTime);
    void deleteEventById(int evtId);

    std::vector<SchedEventTyp> getEventTypes();
    std::vector<SchedProfile> getProfiles();
    std::vector<SchedEvent> getEvents();
    SchedPayload getAllData();

private:
    QSqlQuery executeSqlQuery(const QString& query);

    static std::recursive_mutex mMutex;
    QString mModuleName;
    QSqlDatabase mDatabase;
};

class ScheduleDatabaseFactory
{
public:
    static std::unique_ptr<ScheduleDatabase> createDatabaseConnection(const QString& moduleName = "")
    {
        auto database = std::unique_ptr<ScheduleDatabase>(new ScheduleDatabase(moduleName));
        return database;
    }
};
