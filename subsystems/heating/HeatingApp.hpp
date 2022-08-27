#pragma once

#include <QString>
#include <QObject>
#include <QRunnable>
#include <subsystems/heating/HeatingpAppComponents.hpp>
#include <subsystems/AppBase.hpp>


class HeatingApp : public AppBase
{
    Q_OBJECT
public:
    HeatingApp(QObject* parent, Components* components);
    virtual ~HeatingApp() = default;

    void run();
};
