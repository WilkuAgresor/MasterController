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

    auto queryString = QString("select * from Controllers WHERE name = \"%1\"").arg(name);

    auto query = executeSqlQuery(queryString);

    int idName      = query.record().indexOf("name");
    int idIpAddr    = query.record().indexOf("ipAddr");
    int idPort      = query.record().indexOf("port");
    int idKey    = query.record().indexOf("key");
    int idType      = query.record().indexOf("type");
    int idLastSuccessContactTime = query.record().indexOf("lastSuccTime");
    int idLastTryContactTime = query.record().indexOf("lastTryTime");


    while(query.next())
    {
        info.name = query.value(idName).toString();
        info.ipAddr = query.value(idIpAddr).toString();
        info.port = query.value(idPort).toULongLong();
        info.key = query.value(idKey).toULongLong();
        info.type = static_cast<ControllerInfo::Type>(query.value(idType).toInt());
        info.lastSuccContactTime = QDateTime::fromString(query.value(idLastSuccessContactTime).toString());
        info.lastTryContactTime = QDateTime::fromString(query.value(idLastTryContactTime).toString());
    }

    return info;
}

std::vector<ControllerInfo> Database::getControllers()
{
    std::vector<ControllerInfo> controllers;

    std::ranges::transform(getControllerNames(), std::back_inserter(controllers), [this](const auto& x){return getControllerInfo(x);});

    return controllers;
}

void Database::updateControllerCurKey(const QString& deviceName, uint64_t newKey)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString("UPDATE Controllers SET key = %1 WHERE name = \"%2\"")
                              .arg(QString::number(newKey), deviceName);

    executeSqlQuery(queryString);
}

void Database::updateControllerTryContactTime(const QString& deviceName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto now = QDateTime::currentDateTime();

    auto queryString = QString("UPDATE Controllers SET lastTryTime = %1 WHERE name = \"%2\"")
                              .arg(now.toString(), deviceName);

    executeSqlQuery(queryString);
}

void Database::updateControllerSuccessContactTime(const QString& deviceName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto now = QDateTime::currentDateTime();

    auto queryString = QString("UPDATE Controllers SET lastSuccTime = %1 WHERE name = \"%2\"")
                              .arg(now.toString(), deviceName);

    executeSqlQuery(queryString);
}

void Database::updateControllerStatus(const QString &deviceName, ControllerInfo::Status status)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString("UPDATE Controllers SET status = %1 WHERE name = \"%2\"")
                              .arg(QString::number(static_cast<int>(status)), deviceName);

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

    auto queryString = QString(R"(SELECT controller_name FROM Devices WHERE name="%1")")
                              .arg(deviceName);

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

    auto queryString = QString(R"(SELECT device_name FROM SystemDeviceRelation WHERE system_name="%1")")
                              .arg(systemName);

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

    QString queryString = QString(R"(SELECT device_name FROM RoomDeviceRelation WHERE room_name="%1")")
                              .arg(roomName);

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

    auto queryString = QString(R"(SELECT * FROM Devices WHERE name = "%1")")
                              .arg(deviceName);

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

    std::ranges::transform(getHeatingZoneIds(), std::back_inserter(zones), [this, &profileId](const auto& x){return getHeatZoneSettings(profileId, x);});

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

    auto queryString = QString(R"(SELECT * FROM HeatingZoneGuiSetting WHERE HeatingZoneId=%1)")
                              .arg(zoneId);

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

    return guiSettings;
}

std::vector<HeatingZone> Database::getHeatingZonesHardware(quint16 systemId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);
    std::vector<HeatingZone> zones;

    auto queryString = QString(R"(SELECT *
                                 FROM HeatingZone
                                 WHERE HeatingZone.HeatingSystemId = %1)")
                              .arg(systemId);

    auto query = executeSqlQuery(queryString);

    int idId = query.record().indexOf("HeatingZoneId");

    while(query.next())
    {
        zones.emplace_back(query.value(idId).toInt());
    }
    return zones;
}

std::vector<TemperatureSensor> Database::getTemperatureSensorsHardware(quint16 zoneId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<TemperatureSensor> sensors;

    auto queryString = QString(R"(SELECT *
                                 FROM TemperatureSensors
                                 WHERE TemperatureSensors.zoneId = %1)")
                              .arg(zoneId);

    auto query = executeSqlQuery(queryString);

    int idId = query.record().indexOf("id");
    int idSerial = query.record().indexOf("serial");
    int idType = query.record().indexOf("type");

    while(query.next())
    {
        sensors.emplace_back(
                    TemperatureSensor(
                        query.value(idId).toInt(),
                        query.value(idSerial).toString(),
                        static_cast<TemperatureSensorType>(query.value(idType).toInt())));
    }
    return sensors;
}

LightControllerSettings Database::getLightSetting(int id)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString(R"(SELECT *
                                 FROM LightsControl
                                 LEFT JOIN LightsControlGuiSetting
                                 ON LightsControl.LightsControlId = LightsControlGuiSetting.LightsControlId
                                 WHERE LightsControl.LightsControlId = %1)")
                              .arg(id);

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
    lightSettings.mType = static_cast<LightControllerType>(query.value(idType).toInt());

    auto onVal = query.value(idIsOn).toInt();
    if(onVal == 0) lightSettings.mIsOn = false;
    else lightSettings.mIsOn = true;

    lightSettings.mDimm = query.value(idDimm).toInt();
    lightSettings.mColor = query.value(idColor).toString();
    lightSettings.mGuiSettings.mX = query.value(idX).toInt();
    lightSettings.mGuiSettings.mY = query.value(idY).toInt();
    lightSettings.mGuiSettings.mPlane = query.value(idPlane).toInt();

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
        lightSettings.mType = static_cast<LightControllerType>(query.value(idType).toInt());

        lightSettings.mIsOnChanged = true;
        auto onVal = query.value(idIsOn).toInt();
        if(onVal == 0) lightSettings.mIsOn = false;
        else lightSettings.mIsOn = true;

        lightSettings.mDimmChanged = true;
        lightSettings.mDimm = query.value(idDimm).toInt();
        lightSettings.mColorChanged = true;
        lightSettings.mColor = query.value(idColor).toString();
        lightSettings.mGuiSettingsChanged = true;
        lightSettings.mGuiSettings.mX = query.value(idX).toInt();
        lightSettings.mGuiSettings.mY = query.value(idY).toInt();
        lightSettings.mGuiSettings.mPlane = query.value(idPlane).toInt();

        qDebug() << "light settings: "<<lightSettings.toString();
        lightControllers.push_back(lightSettings);
    }
    return lightControllers;
}

std::vector<RemotePinSetting> Database::getDimmLightSettings(int lightId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);
    std::vector<RemotePinSetting> remoteSettings;

    QString queryString = R"(select RemoteExpanders.pinId, LightsControl.LightsControlDimm, Controllers.ipAddr, Controllers.port from RemoteExpanders
                          LEFT JOIN RemoteDimmLights ON RemoteDimmLights.port = RemoteExpanders.id
                          LEFT JOIN LightsControl ON LightsControl.LightsControlId = RemoteDimmLights.lightId
                          LEFT JOIN Controllers ON RemoteExpanders.controller = Controllers.name
                          WHERE RemoteDimmLights.lightId =
                          )";
    queryString.append(QString::number(lightId));

    auto query = executeSqlQuery(queryString);

    int idControllerIp = query.record().indexOf("ipAddr");
    int idControllerPort = query.record().indexOf("port");
    int idPinId = query.record().indexOf("pinId");
    int idDimm = query.record().indexOf("LightsControlDimm");

    while(query.next())
    {
        RemotePinSetting setting;
        setting.mControllerIpAddr = query.value(idControllerIp).toString();
        setting.mControllerPort = query.value(idControllerPort).toInt();
        setting.mPin = PinIdentifier(query.value(idPinId).toString());
        setting.mValue = query.value(idDimm).toInt();
        remoteSettings.push_back(setting);
    }
    return remoteSettings;
}

std::vector<RemotePinSetting> Database::getRemoteSwitchSettings(int lightId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);
    std::vector<RemotePinSetting> remoteSettings;

    auto queryString = QString(R"(SELECT RemoteExpanders.pinId, Controllers.ipAddr, Controllers.port, RemoteSwitch.onState, RemoteSwitch.offState
                                 FROM RemoteExpanders
                                 LEFT JOIN RemoteSwitch ON RemoteSwitch.port = RemoteExpanders.id
                                 LEFT JOIN LightsControl ON LightsControl.LightsControlId = RemoteSwitch.lightId
                                 LEFT JOIN Controllers ON RemoteExpanders.controller = Controllers.name
                                 WHERE RemoteSwitch.lightId = %1)")
                              .arg(lightId);

    auto query = executeSqlQuery(queryString);

    int idControllerIp = query.record().indexOf("ipAddr");
    int idControllerPort = query.record().indexOf("port");
    int idPinId = query.record().indexOf("pinId");
    int idOnState = query.record().indexOf("onState");
    int idOffState = query.record().indexOf("offState");


    while(query.next())
    {
        RemotePinSetting setting;
        setting.mControllerIpAddr = query.value(idControllerIp).toString();
        setting.mControllerPort = query.value(idControllerPort).toInt();
        setting.mPin = PinIdentifier(query.value(idPinId).toString());
        setting.mOnSetting = query.value(idOnState).toInt();
        setting.mOffSetting = query.value(idOffState).toInt();

        remoteSettings.push_back(setting);
    }
    return remoteSettings;
}

RGBSetting Database::getRGBSetting(int lightId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryStr = QString("SELECT RemoteExpanders.pinId, LightsControl.LightsControlRGB, LightsControl.LightsControlDimm, Controllers.ipAddr, Controllers.port\
                          FROM RemoteExpanders\
                          LEFT JOIN RemoteRGBLights ON RemoteRGBLights.%1 = RemoteExpanders.id\
                          LEFT JOIN LightsControl ON LightsControl.LightsControlId = RemoteRGBLights.lightId\
                          LEFT JOIN Controllers ON RemoteExpanders.controller = Controllers.name\
                          WHERE RemoteRGBLights.lightId = %2");

    RGBSetting colorSetting;

    {
        auto query = executeSqlQuery(queryStr.arg("red", QString::number(lightId)));

        int idControllerIp = query.record().indexOf("ipAddr");
        int idControllerPort = query.record().indexOf("port");
        int idPinId = query.record().indexOf("pinId");

        while(query.next())
        {
            RemotePinSetting setting;
            setting.mControllerIpAddr = query.value(idControllerIp).toString();
            setting.mControllerPort = query.value(idControllerPort).toInt();
            setting.mPin = PinIdentifier(query.value(idPinId).toString());
            colorSetting.mRedPins.push_back(setting);
        }
    }

    {
        auto query = executeSqlQuery(queryStr.arg("green", QString::number(lightId)));

        int idControllerIp = query.record().indexOf("ipAddr");
        int idControllerPort = query.record().indexOf("port");
        int idPinId = query.record().indexOf("pinId");

        while(query.next())
        {
            RemotePinSetting setting;
            setting.mControllerIpAddr = query.value(idControllerIp).toString();
            setting.mControllerPort = query.value(idControllerPort).toInt();
            setting.mPin = PinIdentifier(query.value(idPinId).toString());
            colorSetting.mGreenPins.push_back(setting);
        }
    }

    {       
        auto query = executeSqlQuery(queryStr.arg("blue", QString::number(lightId)));

        int idControllerIp = query.record().indexOf("ipAddr");
        int idControllerPort = query.record().indexOf("port");
        int idPinId = query.record().indexOf("pinId");

        while(query.next())
        {
            RemotePinSetting setting;
            setting.mControllerIpAddr = query.value(idControllerIp).toString();
            setting.mControllerPort = query.value(idControllerPort).toInt();
            setting.mPin = PinIdentifier(query.value(idPinId).toString());
            colorSetting.mBluePins.push_back(setting);
        }
    }

    return colorSetting;
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

        qDebug() <<"adding mapping <lightId:groupingId>: <"<<lightId<<","<<groupingId<<">";

        lightHardwareMapping.emplace(lightId, groupingId);
    }
    return lightHardwareMapping;
}

void Database::setLightsIsOn(int lightId, bool isOn)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    int intValue = isOn ? 1 : 0;

    auto queryString = QString("UPDATE LightsControl SET LightsControlOn = %1 WHERE LightsControlId = %2")
                              .arg(intValue, lightId);

    executeSqlQuery(queryString);
}

void Database::setLightsDimm(int lightId, int dimm)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString("UPDATE LightsControl SET LightsControlDimm = %1 WHERE LightsControlId = %2")
                              .arg(dimm, lightId);

    executeSqlQuery(queryString);
}

void Database::setLightsColor(int lightId, const QString &color)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString(R"(UPDATE LightsControl SET LightsControlRGB = "%1" WHERE LightsControlId = %2)")
                              .arg(color, lightId);

    executeSqlQuery(queryString);
}

void Database::setLightsGuiSettings(int lightId, const LightControllerGuiSettings &guiSettings)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString("UPDATE LightsControlGuiSetting SET %1 = %2 WHERE LightsControlId = %3");

    executeSqlQuery(queryString.arg("x", QString::number(guiSettings.mX), QString::number(lightId)));

    executeSqlQuery(queryString.arg("y", QString::number(guiSettings.mY), QString::number(lightId)));

    executeSqlQuery(queryString.arg("plane", QString::number(guiSettings.mPlane), QString::number(lightId)));
}

PinMapping Database::getLightPinMapping(int lightId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString(R"(SELECT GrandCentralGroupings.GroupingId, GrandCentralGroupings.GroupingInputPins, GrandCentralGroupings.GroupingOutputPins, GrandCentralGroupings.NotifyClick, GrandCentralGroupings.NotifyDoubleClick, GrandCentralGroupings.NotifyPress
                                  FROM GrandCentralGroupings
                                  LEFT JOIN LightsControl
                                  ON LightsControl.HardwareGrouping = GrandCentralGroupings.GroupingId
                                  WHERE LightsControl.LightsControlId = %1)")
                              .arg(lightId);

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
    auto queryString = QString(R"(SELECT GrandCentralGroupings.GroupingId
                                  FROM GrandCentralGroupings
                                  LEFT JOIN LightsControl
                                  ON LightsControl.HardwareGrouping = GrandCentralGroupings.GroupingId
                                  WHERE LightsControl.LightsControlId = %1)")
                              .arg(lightId);

    auto query = executeSqlQuery(queryString);
    int idId = query.record().indexOf("GroupingId");

    query.next();

    return query.value(idId).toInt(0);
}

int Database::getLightIdFromPinId(const PinIdentifier &id)
{
    auto queryString = QString(R"(SELECT LightsControl.LightsControlId
                                  FROM LightsControl
                                  LEFT JOIN GrandCentralGroupings
                                  ON LightsControl.HardwareGrouping = GrandCentralGroupings.GroupingId
                                  WHERE GrandCentralGroupings.GroupingInputPins LIKE '%1')")
                              .arg(id.toStringDatabase());

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

HeatZoneSetting Database::getHeatZoneSettings(int profileId, int zoneId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = QString(R"(SELECT * FROM HeatingZoneSetting WHERE HeatingZoneId = %1 AND HeatingProfileId = %2)")
                              .arg(QString::number(zoneId), QString::number(profileId));

    auto query = executeSqlQuery(queryString);

    int idTemp    = query.record().indexOf("HeatingZoneSettingTemp");
    int idIsOn      = query.record().indexOf("HeatingZoneSettingIsOn");
    int idId      = query.record().indexOf("HeatingZoneId");

    query.next();

    auto temp = query.value(idTemp).toInt();
    auto isOnInt = query.value(idIsOn).toInt();
    auto id = query.value(idId).toInt();

    bool isOn = isOnInt != 0;

    auto guiSettings = getHeatZoneGuiSettings(zoneId);
    auto name = getHeatZoneName(zoneId);

    return HeatZoneSetting(temp, isOn, name, id, guiSettings);
}

std::optional<BoilerSettingsPayload> Database::getBoilerSettings()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);
    QString queryString = constructBoilerDatabaseRetrieveQuery();

    auto query = executeSqlQuery(queryString);

    int idName    = query.record().indexOf("Name");
    int idValue      = query.record().indexOf("Value");

    if(query.size() == 0)
    {
        return {};
    }

    BoilerSettingsPayload boilerSettings;

    while(query.next())
    {
        QString name = query.value(idName).toString();
        auto value = query.record().value(idValue);
        boilerSettings.setPropertyByName(name,value.toInt());
    }
    return boilerSettings;
}

void Database::createBoilerSettingsTable()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    QString queryString = R"(CREATE TABLE BoilerSettings (
    Name TEXT PRIMARY KEY,
    Value INTEGER NOT NULL);)";

    executeSqlQuery(queryString);
}

std::optional<int> Database::getBoilerSettingByName(const QString& name)
{
    //no locking here - internal only
    auto queryString = QString(R"(SELECT Value FROM BoilerSettings WHERE Name="%1")")
                              .arg(name);

    auto query = executeSqlQuery(queryString);

    int idValue      = query.record().indexOf("Value");

    if(query.size() == 0)
    {
        return {};
    }

    BoilerSettingsPayload boilerSettings;

    while(query.next())
    {
        return query.record().value(idValue).toInt();
    }
    return {};
}

void Database::setBoilerSettingByName(const QString &name, int value)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);
    auto curVal = getBoilerSettingByName(name);

    if(!curVal)
    {
        //INSERT INTO BoilerSettings (Name,Value) VALUES ("dupa",5)
        auto queryString = QString(R"(INSERT INTO BoilerSettings (Name, Value) VALUES ("%1", %2))")
                                  .arg(name, QString::number(value));

        executeSqlQuery(queryString);
    }
    else if(value != curVal)
    {
        //UPDATE BoilerSettings SET Value=5 WHERE Name="dupa"
        QString queryString = QString(R"(UPDATE BoilerSettings SET Value=%1 WHERE Name="%2")")
                                  .arg(QString::number(value), name);

        executeSqlQuery(queryString);
    }
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

std::vector<int> Database::getHeatingZoneIds()
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    std::vector<int> ids;
    QString queryString = R"(select HeatingZoneId from HeatingZone)";

    auto query = executeSqlQuery(queryString);

    int idId = query.record().indexOf("HeatingZoneId");

    while(query.next())
    {
        ids.emplace_back(query.value(idId).toInt());
    }
    return ids;
}

HeatZoneSetting Database::getHeatZoneSettings(const QString& profileName, int zoneId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto profileId = getHeatProfileId(profileName);

    return getHeatZoneSettings(profileId, zoneId);
}

int Database::getHeatZoneId(const QString &zoneName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString(R"(SELECT HeatingZoneId FROM HeatingZone WHERE HeatingZoneName="%1")")
                              .arg(zoneName);

    auto query = executeSqlQuery(queryString);
    query.next();
    int idId = query.record().indexOf("HeatingZoneId");
    return query.value(idId).toInt();
}

QString Database::getHeatZoneName(int zoneId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString(R"(SELECT HeatingZoneName FROM HeatingZone WHERE HeatingZoneId=%1)")
                              .arg(zoneId);

    auto query = executeSqlQuery(queryString);
    query.next();
    int idName = query.record().indexOf("HeatingZoneName");
    return query.value(idName).toString();
}

int Database::getHeatProfileId(const QString &profileName)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString(R"(SELECT HeatingProfileId FROM HeatingProfile WHERE HeatingProfileName="%1")")
                              .arg(profileName);

    auto query = executeSqlQuery(queryString);
    query.next();
    int idProfileName = query.record().indexOf("HeatingProfileId");
    return query.value(idProfileName).toInt();
}

QString Database::getHeatProfileName(int profileId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString(R"(SELECT HeatingProfileName FROM HeatingProfile WHERE HeatingProfileId=%1)")
                              .arg(QString::number(profileId));

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

    int intValue = value ? 1: 0;

    auto queryString = QString("UPDATE HeatingSystem SET HeatingMasterIsOn = %1 WHERE HeatingSystemId = 1")
                              .arg(QString::number(intValue));

    executeSqlQuery(queryString);
}

void Database::setHeatZoneTemperature(int temperature, int zoneId, int profileId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    auto queryString = QString("UPDATE HeatingZoneSetting SET HeatingZoneSettingTemp = %1 WHERE HeatingZoneId = %2 AND HeatingProfileId = %3")
                              .arg(QString::number(temperature))
                              .arg(QString::number(zoneId))
                              .arg(QString::number(profileId));

    executeSqlQuery(queryString);
}

void Database::setHeatZoneIsOn(bool value, int zoneId, int profileId)
{
    std::lock_guard<std::recursive_mutex> _lock(mMutex);

    int intValue = value ? 1 : 0;

    auto queryString = QString("UPDATE HeatingZoneSetting SET HeatingZoneSettingIsOn = %1 WHERE HeatingZoneId = %2 AND HeatingProfileId = %3")
                              .arg(QString::number(intValue))
                              .arg(QString::number(zoneId))
                              .arg(QString::number(profileId));

    executeSqlQuery(queryString);
}

