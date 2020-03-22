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

std::vector<HeatZoneSetting> Database::getHeatZoneSettings(int profileId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<HeatZoneSetting> zones;
    for(auto& zoneName: getHeatingZoneNames())
    {
        zones.push_back(getHeatZoneSettings(profileId, zoneName));
    }
    return zones;
}

HeatZoneGuiSettings Database::getHeatZoneGuiSettings(const QString& zoneName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto zoneId = getHeatZoneId(zoneName);
    return getHeatZoneGuiSettings(zoneId);
}

HeatZoneGuiSettings Database::getHeatZoneGuiSettings(int zoneId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(select * from HeatingZoneGuiSetting where HeatingZoneId=)";
    queryString.append(QString::number(zoneId));

    qDebug() << queryString;

    auto query = executeSqlQuery(queryString);

    int idX = query.record().indexOf("x");
    int idY = query.record().indexOf("y");
    int idPlane = query.record().indexOf("plane");
    int idHeight = query.record().indexOf("height");
    int idWidth = query.record().indexOf("width");
    int idFontSize = query.record().indexOf("fontSize");

    query.next();

    HeatZoneGuiSettings guiSettings;
    guiSettings.mX = query.value(idX).toInt();
    guiSettings.mY = query.value(idY).toInt();
    guiSettings.mPlane = query.value(idPlane).toInt();
    guiSettings.mHeight = query.value(idHeight).toInt();
    guiSettings.mWidth = query.value(idWidth).toInt();
    guiSettings.mFontSize = query.value(idFontSize).toInt();

    qDebug() << "heat zone gui settings: "<<guiSettings.toString();

    return guiSettings;
}

LightControllerSettings Database::getLightSetting(int id)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(SELECT *
                          FROM LightsControl
                          LEFT JOIN LightsControlGuiSetting
                          ON LightsControl.LightsControlId = LightsControlGuiSetting.LightsControlId
                          WHERE LightsControl.LightsControlId = )";
    queryString.append(QString::number(id));

    qDebug() << queryString;
    auto query= executeSqlQuery(queryString);

    int idId = query.record().indexOf("LightsControlId");
    int idName = query.record().indexOf("LightsControlName");
    int idType = query.record().indexOf("LightsControlType");
    int idIsOn = query.record().indexOf("LightsControlOn");
    int idDimm = query.record().indexOf("LightsControlDimm");
    int idColor = query.record().indexOf("LightsControlRGB");
    int idX = query.record().indexOf("x");
    int idY = query.record().indexOf("y");
    int idPlane = query.record().indexOf("plane");

    query.next();

    LightControllerSettings lightSettings;
    lightSettings.mId = query.value(idId).toInt();
    lightSettings.mName = query.value(idName).toString();
    lightSettings.mType = query.value(idType).toInt();

    auto onVal = query.value(idIsOn).toInt();
    if(onVal == 0) lightSettings.mIsOn = false;
    else lightSettings.mIsOn = true;

    lightSettings.mDimm = query.value(idDimm).toInt();
    lightSettings.mColor = query.value(idColor).toString();
    lightSettings.mGuiSettings.mX = query.value(idX).toInt();
    lightSettings.mGuiSettings.mY = query.value(idY).toInt();
    lightSettings.mGuiSettings.mPlane = query.value(idPlane).toInt();

    qDebug() << "light settings: "<<lightSettings.toString();

    return lightSettings;

}

std::vector<LightControllerSettings> Database::getLightSettings()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<LightControllerSettings> lightControllers;

    QString queryString = R"(SELECT *
                          FROM LightsControl
                          LEFT JOIN LightsControlGuiSetting
                          ON LightsControl.LightsControlId = LightsControlGuiSetting.LightsControlId)";

    auto query = executeSqlQuery(queryString);

    int idId = query.record().indexOf("LightsControlId");
    int idName = query.record().indexOf("LightsControlName");
    int idType = query.record().indexOf("LightsControlType");
    int idIsOn = query.record().indexOf("LightsControlOn");
    int idDimm = query.record().indexOf("LightsControlDimm");
    int idColor = query.record().indexOf("LightsControlRGB");
    int idX = query.record().indexOf("x");
    int idY = query.record().indexOf("y");
    int idPlane = query.record().indexOf("plane");

    while(query.next())
    {
        LightControllerSettings lightSettings;
        lightSettings.mId = query.value(idId).toInt();
        lightSettings.mName = query.value(idName).toString();
        lightSettings.mType = query.value(idType).toInt();

        auto onVal = query.value(idIsOn).toInt();
        if(onVal == 0) lightSettings.mIsOn = false;
        else lightSettings.mIsOn = true;

        lightSettings.mDimm = query.value(idDimm).toInt();
        lightSettings.mColor = query.value(idColor).toString();
        lightSettings.mGuiSettings.mX = query.value(idX).toInt();
        lightSettings.mGuiSettings.mY = query.value(idY).toInt();
        lightSettings.mGuiSettings.mPlane = query.value(idPlane).toInt();

        qDebug() << "light settings: "<<lightSettings.toString();
        lightControllers.push_back(lightSettings);
    }
    return lightControllers;
}

std::map<int,int> Database::getLightsGroupingMap()
{
    QString queryString = "SELECT LightsControl.LightsControlId, LightsControl.HardwareGrouping from LightsControl";
    auto query = executeSqlQuery(queryString);


    std::map<int,int> lightHardwareMapping;

    int idLightId = query.record().indexOf("LightsControlId");
    int idGroupingId = query.record().indexOf("HardwareGrouping");

    while(query.next())
    {
        int lightId = query.value(idLightId).toInt();
        int groupingId = query.value(idGroupingId).toInt();

        lightHardwareMapping.emplace(lightId, groupingId);
    }
    return lightHardwareMapping;
}

void Database::setLightsIsOn(int lightId, bool isOn)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    int intValue;
    if(isOn)
        intValue = 1;
    else
        intValue = 0;

    QString queryString = "UPDATE LightsControl SET LightsControlOn = ";
    queryString.append(QString::number(intValue));
    queryString.append(R"( WHERE LightsControlId = )");
    queryString.append(QString::number(lightId));

    executeSqlQuery(queryString);
}

void Database::setLightsDimm(int lightId, int dimm)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = "UPDATE LightsControl SET LightsControlDimm = ";
    queryString.append(QString::number(dimm));
    queryString.append(R"( WHERE LightsControlId = )");
    queryString.append(QString::number(lightId));

    executeSqlQuery(queryString);
}

void Database::setLightsColor(int lightId, const QString &color)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(UPDATE LightsControl SET LightsControlRGB = ")";
    queryString.append(color);
    queryString.append(R"(" WHERE LightsControlId = )");
    queryString.append(QString::number(lightId));

    executeSqlQuery(queryString);
}

PinMapping Database::getLightPinMapping(int lightId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(SELECT GrandCentralGroupings.GroupingId, GrandCentralGroupings.GroupingInputPins, GrandCentralGroupings.GroupingOutputPins, GrandCentralGroupings.NotifyClick, GrandCentralGroupings.NotifyDoubleClick, GrandCentralGroupings.NotifyPress
                          FROM GrandCentralGroupings
                          LEFT JOIN LightsControl
                          ON LightsControl.HardwareGrouping = GrandCentralGroupings.GroupingId
                          WHERE LightsControl.LightsControlId = )";
    queryString.append(QString::number(lightId));

    qDebug() << queryString;
    auto query= executeSqlQuery(queryString);

    int idId = query.record().indexOf("GroupingId");
    int idInput = query.record().indexOf("GroupingInputPins");
    int idOutput = query.record().indexOf("GroupingOutputPins");
    int idClick = query.record().indexOf("NotifyClick");
    int idDClick = query.record().indexOf("NotifyDoubleClick");
    int idPress = query.record().indexOf("NotifyPress");

    query.next();

    auto id = query.value(idId).toInt();
    auto input = query.value(idInput).toString();
    auto output = query.value(idOutput).toString();
    auto clickInt = query.value(idClick).toInt();
    auto dClickInt = query.value(idDClick).toInt();
    auto press = query.value(idPress).toInt();

    PinMapping mapping(id);

    mapping.setInputPins(input);
    mapping.setOutputPins(output);
    mapping.setNotifyClick(clickInt>0);
    mapping.setNotifyDoubleClick(dClickInt>0);
    mapping.setNofityPress(press>0);

    return mapping;
}

int Database::getLightGroupingId(int lightId)
{
    QString queryString = R"(SELECT GrandCentralGroupings.GroupingId
                          FROM GrandCentralGroupings
                          LEFT JOIN LightsControl
                          ON LightsControl.HardwareGrouping = GrandCentralGroupings.GroupingId
                          WHERE LightsControl.LightsControlId = )";
    queryString.append(QString::number(lightId));

    auto query = executeSqlQuery(queryString);
    int idId = query.record().indexOf("GroupingId");

    query.next();

    return query.value(idId).toInt(0);
}

int Database::getLightIdFromPinId(const PinIdentifier &id)
{
    QString queryString = R"(SELECT LightsControl.LightsControlId
                          FROM LightsControl
                          LEFT JOIN GrandCentralGroupings
                          ON LightsControl.HardwareGrouping = GrandCentralGroupings.GroupingId
                          WHERE GrandCentralGroupings.GroupingInputPins LIKE )";
    queryString.append(id.toStringDatabase());

    auto query = executeSqlQuery(queryString);
    int idId = query.record().indexOf("LightsControlId");

    query.next();

    return query.value(idId).toInt(0);
}

std::vector<PinMapping> Database::getGrandCentralPinGroupings()
{
    QString queryString = R"(Select * from GrandCentralGroupings)";

    auto query= executeSqlQuery(queryString);

    int idId = query.record().indexOf("GroupingId");
    int idInput = query.record().indexOf("GroupingInputPins");
    int idOutput = query.record().indexOf("GroupingOutputPins");
    int idClick = query.record().indexOf("NotifyClick");
    int idDClick = query.record().indexOf("NotifyDoubleClick");
    int idPress = query.record().indexOf("NotifyPress");

    std::vector<PinMapping> groupings;

    while(query.next())
    {
        auto id = query.value(idId).toInt();
        auto input = query.value(idInput).toString();
        auto output = query.value(idOutput).toString();
        auto clickInt = query.value(idClick).toInt();
        auto dClickInt = query.value(idDClick).toInt();
        auto press = query.value(idPress).toInt();

        PinMapping mapping(id);
        mapping.setInputPins(input);
        mapping.setOutputPins(output);
        mapping.setNotifyClick(clickInt>0);
        mapping.setNotifyDoubleClick(dClickInt>0);
        mapping.setNofityPress(press>0);

        groupings.push_back(mapping);
    }

    return groupings;
}

std::vector<std::pair<PinIdentifier, PinType>> Database::getGrandCentralPins()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(select * from GrandCentralExpanders)";
    auto query = executeSqlQuery(queryString);

    int idId = query.record().indexOf("id");
    int id0 = query.record().indexOf("pin0");
    int id1 = query.record().indexOf("pin1");
    int id2 = query.record().indexOf("pin2");
    int id3 = query.record().indexOf("pin3");
    int id4 = query.record().indexOf("pin4");
    int id5 = query.record().indexOf("pin5");
    int id6 = query.record().indexOf("pin6");
    int id7 = query.record().indexOf("pin7");
    int id8 = query.record().indexOf("pin8");
    int id9 = query.record().indexOf("pin9");
    int id10 = query.record().indexOf("pin10");
    int id11 = query.record().indexOf("pin11");
    int id12 = query.record().indexOf("pin12");
    int id13 = query.record().indexOf("pin13");
    int id14 = query.record().indexOf("pin14");
    int id15 = query.record().indexOf("pin15");

    std::vector<std::pair<PinIdentifier, PinType>> pins;

    while(query.next())
    {
        int expanderId = query.value(idId).toInt();

        qDebug() << "expanderId: "<<expanderId;
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 0), static_cast<PinType>(query.value(id0).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 1), static_cast<PinType>(query.value(id1).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 2), static_cast<PinType>(query.value(id2).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 3), static_cast<PinType>(query.value(id3).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 4), static_cast<PinType>(query.value(id4).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 5), static_cast<PinType>(query.value(id5).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 6), static_cast<PinType>(query.value(id6).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 7), static_cast<PinType>(query.value(id7).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 8), static_cast<PinType>(query.value(id8).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 9), static_cast<PinType>(query.value(id9).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 10), static_cast<PinType>(query.value(id10).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 11), static_cast<PinType>(query.value(id11).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 12), static_cast<PinType>(query.value(id12).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 13), static_cast<PinType>(query.value(id13).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 14), static_cast<PinType>(query.value(id14).toInt(0))));
        pins.push_back(std::make_pair(PinIdentifier(expanderId, 15), static_cast<PinType>(query.value(id15).toInt(0))));
    }

    qDebug() << "pins size: "<<pins.size();

    return pins;
}

HeatZoneSetting Database::getHeatZoneSettings(int profileId, const QString& zoneName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto zoneId = getHeatZoneId(zoneName);

    QString queryString = R"(select * from HeatingZoneSetting where HeatingZoneId=)";
    queryString.append(QString::number(zoneId));
    queryString.append(" AND ");
    queryString.append("HeatingProfileId=");
    queryString.append(QString::number(profileId));
   // queryString.append(R"(")");

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
    auto guiSettings = getHeatZoneGuiSettings(zoneId);
    return HeatZoneSetting(temp,isOn,zoneName, guiSettings);
}

std::vector<HeatProfile> Database::getHeatProfiles()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<HeatProfile> profiles;
    QString queryString = R"(select * from HeatingProfile)";

    auto query = executeSqlQuery(queryString);

    int idId    = query.record().indexOf("HeatingProfileId");
    int idName      = query.record().indexOf("HeatingProfileName");

    while(query.next())
    {
        HeatProfile profile;
        profile.mId = query.value(idId).toInt();
        profile.mName = query.value(idName).toString();
        profiles.push_back(profile);
    }

    return profiles;
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

void Database::setHeatMasterOn(bool value)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    int intValue;
    if(value)
        intValue = 1;
    else
        intValue = 0;

    QString queryString = "UPDATE HeatingSystem SET HeatingMasterIsOn = ";
    queryString.append(QString::number(intValue));
    queryString.append(R"( WHERE HeatingSystemId = 1)");

    executeSqlQuery(queryString);
}

void Database::setHeatZoneTemperature(int temperature, int zoneId, int profileId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = "UPDATE HeatingZoneSetting SET HeatingZoneSettingTemp = ";
    queryString.append(QString::number(temperature));
    queryString.append(R"( WHERE HeatingZoneId = )");
    queryString.append(QString::number(zoneId));
    queryString.append(R"( AND HeatingProfileId = )");
    queryString.append(QString::number(profileId));

    executeSqlQuery(queryString);
}

void Database::setHeatZoneIsOn(bool value, int zoneId, int profileId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    int intValue;
    if(value)
        intValue = 1;
    else
        intValue = 0;

    QString queryString = "UPDATE HeatingZoneSetting SET HeatingZoneSettingIsOn = ";
    queryString.append(QString::number(intValue));
    queryString.append(R"( WHERE HeatingZoneId = )");
    queryString.append(QString::number(zoneId));
    queryString.append(R"( AND HeatingProfileId = )");
    queryString.append(QString::number(profileId));

    executeSqlQuery(queryString);
}

