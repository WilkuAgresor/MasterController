#pragma once

#include <QString>
#include <QObject>
#include <QRunnable>
#include <subsystems/schedule/ScheduleAppComponents.hpp>
#include <../common/subsystems/AppBase.hpp>

class Components;

class ScheduleApp : public AppBase
{
    Q_OBJECT
public:
    ScheduleApp(QObject* parent, Components* components);
    virtual ~ScheduleApp() = default;

    void run();
};
