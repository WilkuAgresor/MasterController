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

    QString queryString = R"(CREATE TABLE IF NOT EXISTS ')";
    queryString.append(serialNumber);
    queryString.append(R"(' (
                       timestamp TEXT,
                       value INTEGER
                   ); )");

    qDebug() << queryString;

    executeSqlQuery(queryString);
}

void SensorDatabase::insertRecord(const QString &serialNumber, const QString &timestamp, qint16 value)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(INSERT INTO ')";
    queryString.append(serialNumber);
    queryString.append(R"(' (timestamp, value) VALUES (')");
    queryString.append(timestamp);
    queryString.append(R"(', )");
    queryString.append(QString::number(value));
    queryString.append(R"();)");

    qDebug() << queryString;

    executeSqlQuery(queryString);

}

void SensorDatabase::deleteRecordsByDate(const QString &serialNumber, const QString &date)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(DELETE FROM ')";
    queryString.append(serialNumber);
    queryString.append(R"(' WHERE timestamp REGEXP ')");
    queryString.append(date);
    queryString.append(R"(\S*';")");

    executeSqlQuery(queryString);
}

std::vector<TemperatureSensorDataEntry> SensorDatabase::getRecords(const QString &serialNumber, quint16 limit, quint16 offset)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<TemperatureSensorDataEntry> entries;

    QString queryString = R"(SELECT * FROM ')";
    queryString.append(serialNumber);
    queryString.append(R"(' ORDER BY timestamp LIMIT )");
    queryString.append(QString::number(limit));
    queryString.append(" OFFSET ");
    queryString.append(QString::number(offset));
    queryString.append(R"(;)");

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
