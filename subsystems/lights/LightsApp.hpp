#pragma once

#include <QString>
#include <QObject>
#include <QRunnable>
#include <subsystems/lights/LightsAppComponents.hpp>
#include <../common/subsystems/AppBase.hpp>


class LightsApp : public AppBase
{
    Q_OBJECT
public:
    LightsApp(QObject* parent, Components* components);
    virtual ~LightsApp() = default;

    void run();
};
