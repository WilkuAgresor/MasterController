#include "ScheduleDatabase.hpp"

std::recursive_mutex ScheduleDatabase::mMutex;

ScheduleDatabase::ScheduleDatabase(const QString &moduleName)
    : mModuleName(moduleName)
{
    if(moduleName.isEmpty())
        mDatabase = QSqlDatabase::addDatabase("QSQLITE");
    else
        mDatabase = QSqlDatabase::addDatabase("QSQLITE", moduleName);


    mDatabase.setDatabaseName("/opt/homeAuto/database/sensorDb.db");

    if(!mDatabase.open())
    {
        qDebug() << "Sensor database connection failed to open: "<< moduleName;
    }
}

ScheduleDatabase::~ScheduleDatabase()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    if(mDatabase.isOpen() && !mModuleName.isEmpty())
    {
        mDatabase.close();
        mDatabase = QSqlDatabase();

        QSqlDatabase::removeDatabase(mModuleName);
    }
}

void ScheduleDatabase::insertEvent(int evtType, const QString& body, int intervalProfile, const QString& triggerTime)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString(R"(INSERT INTO scheduledEvents (type, body, intervalProfile, triggerTime)
                                  VALUES (%1, '%2', %3, '%4'))")
                              .arg(QString::number(evtType))
                              .arg(body)
                              .arg(QString::number(intervalProfile))
                              .arg(triggerTime);

    executeSqlQuery(queryString);
}

void ScheduleDatabase::deleteEventById(int evtId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString(R"(DELETE FROM scheduledEvents WHERE id = %1)")
                              .arg(QString::number(evtId));

    executeSqlQuery(queryString);
}

std::vector<SchedEventTyp> ScheduleDatabase::getEventTypes()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);
    std::vector<SchedEventTyp> eventTypes;

    QString queryString = R"(SELECT * FROM eventTypes)";

    auto query = executeSqlQuery(queryString);

    int idType = query.record().indexOf("type");
    int idDescription = query.record().indexOf("description");

    while(query.next())
    {
        eventTypes.emplace_back(query.value(idType).toInt(), query.value(idDescription).toString());
    }
    return eventTypes;
}

std::vector<SchedProfile> ScheduleDatabase::getProfiles()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);
    std::vector<SchedProfile> profiles;

    QString queryString = R"(SELECT * FROM scheduleProfiles)";

    auto query = executeSqlQuery(queryString);

    int idId = query.record().indexOf("id");
    int idInterval = query.record().indexOf("intervalTime");
    int idDescription = query.record().indexOf("description");

    while(query.next())
    {
        profiles.emplace_back(query.value(idId).toInt(), query.value(idInterval).toInt(), query.value(idDescription).toString());
    }
    return profiles;
}

std::vector<SchedEvent> ScheduleDatabase::getEvents()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);
    std::vector<SchedEvent> events;

    QString queryString = R"(SELECT * FROM scheduledEvents)";

    auto query = executeSqlQuery(queryString);

    int idId = query.record().indexOf("id");
    int idType = query.record().indexOf("type");
    int idBody = query.record().indexOf("body");
    int idIntervalProfile = query.record().indexOf("intervalProfile");
    int idTrigger = query.record().indexOf("triggerTime");

    while(query.next())
    {
        events.emplace_back(query.value(idId).toInt(), query.value(idType).toInt(), query.value(idBody).toString(), query.value(idIntervalProfile).toInt(), query.value(idTrigger).toString());
    }
    return events;
}

SchedPayload ScheduleDatabase::getAllData()
{
    SchedPayload payload;
    payload.mTypes = getEventTypes();
    payload.mEvents = getEvents();
    payload.mProfiles = getProfiles();
    payload.mOperation = ScheduleOperationType::RETRIEVE;

    return payload;
}

QSqlQuery ScheduleDatabase::executeSqlQuery(const QString &query)
{
    //qDebug() << "DB query: "<< query;

    if(mModuleName.isEmpty())
    {
        return QSqlQuery(query);
    }
    else
    {
        return QSqlQuery(query, mDatabase);
    }
}
