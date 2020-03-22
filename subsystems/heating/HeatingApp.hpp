#pragma once

#include <QString>
#include <QObject>
#include <QRunnable>
#include <subsystems/heating/HeatingpAppComponents.hpp>



class HeatingApp : public QObject , public QRunnable
{
    Q_OBJECT
public:
    HeatingApp(QObject* parent, Components* components);
    ~HeatingApp() = default;

    void run();

    HeatingAppComponents mComponents;
};
