#pragma once

#include <QtSql>
#include <database/devicedb.hpp>
#include "../common/json/deviceJson.hpp"
#include <../common/subsystems/heating/HeatingZoneSettings.hpp>

struct ControllerInfo
{
    enum Type : int
    {
        RPI_3Bp = 0,
        ARD_LEO = 1,
        TERMINALv1 = 2
    };

    enum Status : int
    {
        INACTIVE = 0,
        ACTIVE   = 1
    };

    QString name;
    QString ipAddr;
    uint64_t port;
    uint64_t key;
    Type type;
    Status status;



    QString print()
    {
        return "name="+name+" ipAddr="+ ipAddr+" port="+ QString::number(port)+" key="+QString::number(key) + " type="+QString::number(static_cast<int>(type))
                 + " status="+QString::number(static_cast<int>(status));
    }
};

class Database
{
public:
    Database(const QString& moduleName = "");
    ~Database();
    Database(const Database&) = delete;


    ControllerInfo getControllerInfo(const QString& name);
    std::vector<ControllerInfo> getControllers();
    void updateControllerCurKey(const QString &deviceName, uint64_t newKey);
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

    HeatZoneSetting getHeatZoneSettings(const QString &profileName, const QString &zoneName);
    HeatZoneSetting getHeatZoneSettings(int profileId, const QString &zoneName);

    std::vector<QString> getHeatingZoneNames();

    int getHeatZoneId(const QString& zoneName);
    int getHeatProfileId(const QString& profileName);

    QString getHeatProfileName(int profileId);
    bool getHeatMasterOn();
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
        auto database = std::unique_ptr<Database>(new Database(moduleName));
        return database;
    }
};
