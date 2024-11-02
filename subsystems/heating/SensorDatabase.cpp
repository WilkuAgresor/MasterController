#include "SensorDatabase.hpp"

std::recursive_mutex SensorDatabase::mMutex;

SensorDatabase::SensorDatabase(const QString &moduleName)
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

SensorDatabase::~SensorDatabase()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    if(mDatabase.isOpen() && !mModuleName.isEmpty())
    {
        mDatabase.close();
        mDatabase = QSqlDatabase();

        QSqlDatabase::removeDatabase(mModuleName);
    }
}

void SensorDatabase::addSensorTableIfNoExist(const QString &serialNumber)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString(R"(CREATE TABLE IF NOT EXISTS '%1' (
                       timestamp TEXT,
                       value INTEGER
                   ); )").arg(serialNumber);


    executeSqlQuery(queryString);
}

void SensorDatabase::insertRecord(const QString &serialNumber, const QString &timestamp, qint16 value)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = QString(R"(INSERT INTO '%1' (timestamp, value) VALUES ('%2', %3);)")
                              .arg(serialNumber, timestamp, QString::number(value));

    executeSqlQuery(queryString);
}

void SensorDatabase::deleteRecordsByDate(const QString &serialNumber, const QString &date)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = QString(R"(DELETE FROM '%1' WHERE timestamp REGEXP '%2\\S*';)")
                              .arg(serialNumber, date);

    executeSqlQuery(queryString);
}

std::vector<TemperatureSensorDataEntry> SensorDatabase::getRecords(const QString &serialNumber, quint16 limit, quint16 offset)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<TemperatureSensorDataEntry> entries;

    QString queryString = QString(R"(SELECT * FROM '%1' ORDER BY timestamp LIMIT %2 OFFSET %3;)")
                              .arg(serialNumber, QString::number(limit), QString::number(offset));

    auto query = executeSqlQuery(queryString);

    int idTimestamp = query.record().indexOf("timestamp");
    int idValue = query.record().indexOf("value");

    while(query.next())
    {
        entries.emplace_back(TemperatureSensorDataEntry(
                                  query.value(idTimestamp).toString(),
                                  query.value(idValue).toInt()
                                  ));

    }
    return entries;
}

QSqlQuery SensorDatabase::executeSqlQuery(const QString &query)
{
    qDebug() << "DB query: "<< query;

    if(mModuleName.isEmpty())
    {
        return QSqlQuery(query);
    }
    else
    {
        return QSqlQuery(query, mDatabase);
    }
}
