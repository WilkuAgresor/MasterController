QT += quick
QT += sql
QT += serialport
QT += core5compat

CONFIG += c++20


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS


# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../common/NetworkPortPool.cpp \
    ../common/messages/partedMessage.cpp \
    ../common/subsystems/AppBase.cpp \
    ../common/subsystems/heating/BoilerSettingsMessage.cpp \
    ../common/subsystems/heating/HeatingRetrieveMessage.cpp \
    ../common/subsystems/heating/HeatingRetrieveStatistics.cpp \
    ../common/subsystems/heating/HeatingZoneSettings.cpp \
    ../common/subsystems/lights/LightControllerSettings.cpp \
    ../common/subsystems/lights/LightsRetrieveMessage.cpp \
    ../common/subsystems/schedule/ScheduleEventTypes.cpp \
    ../common/subsystems/status/ControllerStatusMessage.cpp \
    ../common/subsystems/topology/topologyMessages.cpp \
    LeonardoIpExecutor/LeoMessage.cpp \
    LeonardoIpExecutor/OpenthermMessages.cpp \
    LeonardoIpExecutor/RemoteRGBSetting.cpp \
    database/ScheduleDatabase.cpp \
    hardware/GrandCentral.cpp \
    hardware/Pin.cpp \
    hardware/PinIdentifier.cpp \
    hardware/PinMapping.cpp \
        main.cpp \
    database/database.cpp \
    mainapplication.cpp \
    ../common/json/deviceJson.cpp \
    ../common/json/commonEntity.cpp \
    ../common/sender.cpp \
    ../common/receiver/task.cpp \
    ../common/receiver/receiver.cpp \
    ../common/json/requestJson.cpp \
    ../common/messages/headerJson.cpp \
    ../common/messages/messageCommon.cpp \
    ../common/messages/message_old.cpp \
    ../common/subsystems/alarm/alarmSetMessage.cpp \
    ../common/json/jsonSerialization.cpp \
    ../common/messages/replyMessage.cpp \
    serialListener/SerialConnection.cpp \
    subsystems/heating/HeatingAppComponents.cpp \
    subsystems/heating/SensorDatabase.cpp \
    subsystems/lights/LightsApp.cpp \
    subsystems/lights/LightsAppComponents.cpp \
    subsystems/schedule/ScheduleApp.cpp \
    subsystems/schedule/ScheduleAppComponents.cpp \
    subsystems/schedule/Timer.cpp \
    subsystems/topology/TopologyApp.cpp \
    terminalListener/terminalListener.cpp \
    terminalListener/messageHandler.cpp \
    connection/connectionmonitor.cpp \
    connection/connectiontimer.cpp \
    components.cpp \
    subsystems/heating/HeatingApp.cpp \
    subsystems/heating/SensorData.cpp

#RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ../common/NetworkPortPool.hpp \
    ../common/messages/partedMessage.hpp \
    ../common/subsystems/AppBase.hpp \
    ../common/subsystems/heating/BoilerSettingsMessage.hpp \
    ../common/subsystems/heating/HeatingRetrieveMessage.hpp \
    ../common/subsystems/heating/HeatingRetrieveStatistics.hpp \
    ../common/subsystems/heating/HeatingZoneSettings.hpp \
    ../common/subsystems/lights/LightControllerSettings.hpp \
    ../common/subsystems/lights/LightsDictionary.hpp \
    ../common/subsystems/lights/LightsRetrieveMessage.hpp \
    ../common/subsystems/schedule/ScheduleDictionary.hpp \
    ../common/subsystems/schedule/ScheduleEventTypes.hpp \
    ../common/subsystems/status/ControllerInfo.hpp \
    ../common/subsystems/status/ControllerStatusMessage.hpp \
    ../common/subsystems/topology/topologyMessages.hpp \
    LeonardoIpExecutor/LeoMessage.hpp \
    LeonardoIpExecutor/OpenthermMessages.hpp \
    LeonardoIpExecutor/RemoteRGBSetting.hpp \
    connection/connectiontimer.hpp \
    connection/connectionmonitor.hpp \
    database/ScheduleDatabase.hpp \
    database/database.hpp \
    database/devicedb.hpp \
    hardware/GrandCentral.hpp \
    hardware/Pin.hpp \
    hardware/PinIdentifier.hpp \
    hardware/PinMapping.hpp \
    mainapplication.hpp \
    ../common/json/deviceJson.hpp \
    ../common/json/commonEntity.hpp \
    ../common/sender.hpp \
    ../common/receiver/task.hpp \
    ../common/receiver/receiver.hpp \
    ../common/json/requestJson.hpp \
    ../common/json/nicknames.hpp \
    ../common/messages/message_old.hpp \
    ../common/messages/messageCommon.hpp \
    ../common/messages/headerJson.hpp \
    ../common/subsystems/alarm/alarmSetMessage.hpp \
    ../common/subsystems/heating/HeatingDictionary.hpp \
    ../common/json/jsonSerialization.hpp \
    ../common/messages/replyMessage.hpp \
    ../common/CommonDefinitions.hpp \
    components.hpp \
    serialListener/SerialConnection.hpp \
    subsystems/heating/HeatingAppComponents.hpp \
    subsystems/heating/SensorDatabase.hpp \
    subsystems/lights/LightsApp.hpp \
    subsystems/lights/LightsAppComponents.hpp \
    subsystems/schedule/ScheduleApp.hpp \
    subsystems/schedule/ScheduleAppComponents.hpp \
    subsystems/schedule/Timer.hpp \
    subsystems/topology/TopologyApp.hpp \
    terminalListener/terminalListener.hpp \
    terminalListener/messageHandler.hpp \
    subsystems/heating/HeatingApp.hpp \
    subsystems/heating/SensorData.hpp

