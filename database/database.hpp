#pragma once

#include <QtSql>
#include <database/devicedb.hpp>
#include "../common/json/deviceJson.hpp"
#include <../common/subsystems/heating/HeatingZoneSettings.hpp>
#include <../common/subsystems/heating/BoilerSettingsMessage.hpp>
#include <../common/subsystems/lights/LightControllerSettings.hpp>
#include <hardware/PinMapping.hpp>
#include <../common/subsystems/status/ControllerInfo.hpp>
#include <subsystems/heating/SensorData.hpp>
#include <LeonardoIpExecutor/RemoteRGBSetting.hpp>

class Database
{
public:
    Database(const QString& moduleName = "");
    ~Database();
    Database(const Database&) = delete;


    ControllerInfo getControllerInfo(const QString& name);
    std::vector<ControllerInfo> getControllers();
    void updateControllerCurKey(const QString &deviceName, uint64_t newKey);
    void updateControllerTryContactTime(const QString& deviceName);
    void updateControllerSuccessContactTime(const QString& deviceName);
    void updateControllerStatus(const QString& deviceName, ControllerInfo::Status status);
    std::vector<QString> getControllerNames();

    std::vector<QString> getDeviceNames();
    QString getDevicesControllerName(const QString& deviceName);
    ControllerInfo getDevicesControllerInfo(const QString& deviceName);

    std::vector<QString> getSystemNames();
    std::vector<QString> getSystemDeviceNames(const QString& systemName);

    std::vector<QString> getRoomNames();
    std::vector<QString> getRoomDeviceNames(const QString& roomName);

    DeviceInfo getDeviceInfo(const QString& deviceName);
    DeviceInfo getDeviceInfoHw(const QString& hardwareId);

    std::vector<HeatZoneSetting> getHeatZoneSettings(int profileId);
    HeatZoneSetting getHeatZoneSettings(const QString &profileName, int zoneId);
    HeatZoneSetting getHeatZoneSettings(int profileId, int zoneId);

    void createBoilerSettingsTable();
    std::optional<BoilerSettingsPayload> getBoilerSettings();
    std::optional<int> getBoilerSettingByName(const QString& name);
    void setBoilerSettingByName(const QString& name, int value);

    std::vector<int> getHeatingZoneIds();

    int getHeatZoneId(const QString& zoneName);
    QString getHeatZoneName(int zoneId);
    int getHeatProfileId(const QString& profileName);

    QString getHeatProfileName(int profileId);
    std::vector<HeatProfile> getHeatProfiles();

    bool getHeatMasterOn();    
    void setHeatMasterOn(bool value);

    void setHeatZoneTemperature(int temperature, int zoneId, int profileId);
    void setHeatZoneIsOn(bool value, int zoneId, int profileId);
    HeatZoneGuiSettings getHeatZoneGuiSettings(const QString &zoneName);
    HeatZoneGuiSettings getHeatZoneGuiSettings(int zoneId);

    std::vector<HeatingZone> getHeatingZonesHardware(quint16 systemId = 1);
    std::vector<TemperatureSensor> getTemperatureSensorsHardware(quint16 zoneId);

    LightControllerSettings getLightSetting(int id);
    std::vector<LightControllerSettings> getLightSettings();

    std::vector<RemotePinSetting> getDimmLightSettings(int lightId);
    RGBSetting getRGBSetting(int lightId);

    std::vector<RemotePinSetting> getRemoteSwitchSettings(int lightId);

    void setLightsIsOn(int lightId, bool isOn);
    void setLightsDimm(int lightId, int dimm);
    void setLightsColor(int lightId, const QString& color);
    void setLightsGuiSettings(int lightId, const LightControllerGuiSettings& guiSettings);

    PinMapping getLightPinMapping(int lightId);
    int getLightGroupingId(int lightId);
    std::map<int, int> getLightsGroupingMap();

    int getLightIdFromPinId(const PinIdentifier& id);

    std::vector<PinMapping> getGrandCentralPinGroupings();
    std::vector<std::pair<PinIdentifier, PinType> > getGrandCentralPins();

private:
    QSqlQuery executeSqlQuery(const QString& query);

    static std::recursive_mutex mMutex;
    QString mModuleName;
    QSqlDatabase mDatabase;
};

class DatabaseFactory
{
public:
    static std::unique_ptr<Database> createDatabaseConnection(const QString& moduleName = "")
    {
        qDebug() << "creating database connection:"<< moduleName;
        auto database = std::unique_ptr<Database>(new Database(moduleName));
        return database;
    }
};

