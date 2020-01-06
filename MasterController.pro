QT += quick
QT += sql
CONFIG += c++14

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
    ../common/ResponseReceiver.cpp \
    ../common/subsystems/heating/HeatingZoneSettings.cpp \
    ../common/subsystems/topology/topologyMessages.cpp \
        main.cpp \
    database/database.cpp \
    mainapplication.cpp \
    ../common/simplecrypt/simplecrypt.cpp \
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
    subsystems/heating/HeatingpAppComponents.cpp \
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
    ../common/ResponseReceiver.hpp \
    ../common/subsystems/heating/HeatingZoneSettings.hpp \
    ../common/subsystems/topology/topologyMessages.hpp \
    connection/connectiontimer.hpp \
    connection/connectionmonitor.hpp \
    database/database.hpp \
    database/devicedb.hpp \
    mainapplication.hpp \
    ../common/simplecrypt/simplecrypt.hpp \
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
    subsystems/heating/HeatingpAppComponents.hpp \
    subsystems/topology/TopologyApp.hpp \
    terminalListener/terminalListener.hpp \
    terminalListener/messageHandler.hpp \
    subsystems/heating/HeatingApp.hpp \
    subsystems/heating/SensorData.hpp

