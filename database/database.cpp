#include <database/database.hpp>

std::recursive_mutex Database::mMutex;

Database::Database(const QString &moduleName)
    : mModuleName(moduleName)
{
    if(moduleName.isEmpty())
        mDatabase = QSqlDatabase::addDatabase("QSQLITE");
    else
        mDatabase = QSqlDatabase::addDatabase("QSQLITE", moduleName);


    mDatabase.setDatabaseName("/opt/homeAuto/database/dbMain1.db");

    if(!mDatabase.open())
    {
        qDebug() << "Database connection failed to open: "<< moduleName;
    }
}

Database::~Database()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    if(mDatabase.isOpen() && !mModuleName.isEmpty())
    {
        mDatabase.close();
        mDatabase = QSqlDatabase();

        QSqlDatabase::removeDatabase(mModuleName);
    }
}

QSqlQuery Database::executeSqlQuery(const QString &query)
{
    if(mModuleName.isEmpty())
    {
        return QSqlQuery(query);
    }
    else
    {
        return QSqlQuery(query, mDatabase);
    }
}

std::vector<QString> Database::getControllerNames()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<QString> controllers;

    auto query = executeSqlQuery("select name from Controllers");

    int idName = query.record().indexOf("name");

    while(query.next())
    {
        controllers.emplace_back(query.value(idName).toString());
    }
    return controllers;
}

ControllerInfo Database::getControllerInfo(const QString &name)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);
    ControllerInfo info;

    QString queryString = R"(select * from Controllers WHERE name = ")";
    queryString.append(name);
    queryString.append(R"(")");

    auto query = executeSqlQuery(queryString);

    int idName      = query.record().indexOf("name");
    int idIpAddr    = query.record().indexOf("ipAddr");
    int idPort      = query.record().indexOf("port");
    int idKey    = query.record().indexOf("key");
    int idType      = query.record().indexOf("type");

    while(query.next())
    {
        info.name = query.value(idName).toString();
        info.ipAddr = query.value(idIpAddr).toString();
        info.port = query.value(idPort).toULongLong();
        info.key = query.value(idKey).toULongLong();
        info.type = static_cast<ControllerInfo::Type>(query.value(idType).toInt());
    }

    return info;
}

std::vector<ControllerInfo> Database::getControllers()
{
    std::vector<ControllerInfo> controllers;

    auto controllerNames = getControllerNames();
    for(auto& controllerName: controllerNames)
    {
        auto controllerInfo = getControllerInfo(controllerName);
        controllers.push_back(controllerInfo);
    }
    return controllers;
}

void Database::updateControllerCurKey(const QString& deviceName, uint64_t newKey)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = "UPDATE Controllers SET key = ";
    queryString.append(QString::number(newKey));
    queryString.append(R"( WHERE name = ")");
    queryString.append(deviceName);
    queryString.append(R"(")");

    executeSqlQuery(queryString);
}

void Database::updateControllerStatus(const QString &deviceName, ControllerInfo::Status status)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = "UPDATE Controllers SET status = ";
    queryString.append(QString::number(static_cast<int>(status)));
    queryString.append(R"( WHERE name = ")");
    queryString.append(deviceName);
    queryString.append(R"(")");

    executeSqlQuery(queryString);
}

std::vector<QString> Database::getDeviceNames()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<QString> devices;

    auto query = executeSqlQuery("select name from Devices");

    int idName = query.record().indexOf("name");

    while(query.next())
    {
        devices.emplace_back(query.value(idName).toString());
    }
    return devices;
}

QString Database::getDevicesControllerName(const QString &deviceName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(select controller_name from Devices where name=")";
    queryString.append(deviceName);
    queryString.append(R"(")");

    auto query = executeSqlQuery(queryString);

    query.next();
    int idContName = query.record().indexOf("controller_name");

    return query.value(idContName).toString();
}

ControllerInfo Database::getDevicesControllerInfo(const QString &deviceName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    return getControllerInfo(getDevicesControllerName(deviceName));
}

std::vector<QString> Database::getSystemNames()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<QString> systems;

    auto query = executeSqlQuery("select name from Systems");

    int idName = query.record().indexOf("name");

    while(query.next())
    {
        systems.emplace_back(query.value(idName).toString());
    }
    return systems;
}

std::vector<QString> Database::getSystemDeviceNames(const QString &systemName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<QString> devices;
    QString queryString = R"(select device_name from SystemDeviceRelation where system_name=")";
    queryString.append(systemName);
    queryString.append(R"(")");

    auto query = executeSqlQuery(queryString);

    int idName = query.record().indexOf("device_name");

    while(query.next())
    {
        devices.emplace_back(query.value(idName).toString());
    }
    return devices;
}

std::vector<QString> Database::getRoomNames()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<QString> rooms;

    auto query = executeSqlQuery("select name from Rooms");

    int idName = query.record().indexOf("name");

    while(query.next())
    {
        rooms.emplace_back(query.value(idName).toString());
    }
    return rooms;
}

std::vector<QString> Database::getRoomDeviceNames(const QString &roomName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<QString> devices;
    QString queryString = R"(select device_name from RoomDeviceRelation where room_name=")";
    queryString.append(roomName);
    queryString.append(R"(")");

    auto query = executeSqlQuery(queryString);

    int idName = query.record().indexOf("device_name");

    while(query.next())
    {
        devices.emplace_back(query.value(idName).toString());
    }
    return devices;
}

DeviceInfo Database::getDeviceInfo(const QString &deviceName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(select * from Devices WHERE name = ")";
    queryString.append(deviceName);
    queryString.append(R"(")");
    auto query = executeSqlQuery(queryString);

    int idName      = query.record().indexOf("name");
    int idControllerName    = query.record().indexOf("controller_name");
    int idType      = query.record().indexOf("type");
    int idState    = query.record().indexOf("state");
    int idThreshold    = query.record().indexOf("threshold");
    int idValue   = query.record().indexOf("value");
    int idUpDown    = query.record().indexOf("upDown");
    int idHardwareId    = query.record().indexOf("hardware_id");
    int idTrigger = query.record().indexOf("trigger");

    //TODO: this was in a while loop - check if it works without it
    query.next();

    auto name = query.value(idName).toString();
    auto controllerName = query.value(idControllerName).toString();
    auto type = query.value(idType).toString();
    auto state = query.value(idState).toInt();
    auto threshold = query.value(idThreshold).toDouble();
    auto value = query.value(idValue).toDouble();
    auto upDown = query.value(idUpDown).toInt();
    auto hardwareId = query.value(idHardwareId).toString();
    auto trigger = query.value(idTrigger).toInt();

    return DeviceInfo{name, controllerName, type, state, threshold, value, upDown, hardwareId, trigger};
}

HeatZoneSetting Database::getHeatZoneSettings(int profileId, const QString& zoneName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto zoneId = getHeatZoneId(zoneName);

    qDebug() << "profileId: "<< profileId <<" zone id: "<<zoneId;

    QString queryString = R"(select * from HeatingZoneSetting where HeatingZoneId=)";
    queryString.append(QString::number(zoneId));
    queryString.append(" AND ");
    queryString.append("HeatingProfileId=");
    queryString.append(QString::number(profileId));
   // queryString.append(R"(")");

    qDebug() << "query: "<<queryString;

    auto query = executeSqlQuery(queryString);

    int idTemp    = query.record().indexOf("HeatingZoneSettingTemp");
    int idIsOn      = query.record().indexOf("HeatingZoneSettingIsOn");

    query.next();

    auto temp = query.value(idTemp).toInt();
    auto isOnInt = query.value(idIsOn).toInt();

    bool isOn;
    if(isOnInt == 0)
    {
        isOn = false;
    }
    else
    {
        isOn = true;
    }

    return HeatZoneSetting(temp,isOn,zoneName);
}

std::vector<QString> Database::getHeatingZoneNames()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<QString> names;
    QString queryString = R"(select HeatingZoneName from HeatingZone)";

    auto query = executeSqlQuery(queryString);

    int idName = query.record().indexOf("HeatingZoneName");

    while(query.next())
    {
        names.emplace_back(query.value(idName).toString());
    }
    return names;
}

HeatZoneSetting Database::getHeatZoneSettings(const QString& profileName, const QString& zoneName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto profileId = getHeatProfileId(profileName);

    return getHeatZoneSettings(profileId, zoneName);
}

int Database::getHeatZoneId(const QString &zoneName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(select HeatingZoneId from HeatingZone where HeatingZoneName=")";
    queryString.append(zoneName);
    queryString.append(R"(")");

    QSqlQuery query(queryString, mDatabase);
    query.next();
    int idZoneName = query.record().indexOf("HeatingZoneId");
    return query.value(idZoneName).toInt();
}

int Database::getHeatProfileId(const QString &profileName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(select HeatingProfileId from HeatingProfile where HeatingProfileName=")";
    queryString.append(profileName);
    queryString.append(R"(")");

    auto query = executeSqlQuery(queryString);
    query.next();
    int idProfileName = query.record().indexOf("HeatingProfileId");
    return query.value(idProfileName).toInt();
}

QString Database::getHeatProfileName(int profileId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(select HeatingProfileName from HeatingProfile where HeatingProfileId=)";
    queryString.append(QString::number(profileId));
    qDebug() << "query: "<<queryString;

    auto query = executeSqlQuery(queryString);
    query.next();
    int idProfileName = query.record().indexOf("HeatingProfileName");
    return query.value(idProfileName).toString();
}

bool Database::getHeatMasterOn()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(select HeatingMasterIsOn from HeatingSystem where HeatingSystemId=1)";

    auto query = executeSqlQuery(queryString);
    query.next();
    int idProfileName = query.record().indexOf("HeatingMasterIsOn");
    if(query.value(idProfileName).toInt() == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}
