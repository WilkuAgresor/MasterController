#pragma once

#include <QObject>
#include <database/database.hpp>
#include <../common/sender.hpp>
#include <subsystems/heating/HeatingpAppComponents.hpp>
#include <subsystems/lights/LightsAppComponents.hpp>

class Components : public QObject
{
public:
    Components(QObject* parent);
    ~Components() = default;


    Sender* mSender;
    std::vector<ControllerInfo> mControllers;
    std::unique_ptr<HeatingAppComponents> mHeatingComponents;
    std::unique_ptr<LightsAppComponents> mLightsComponents;

};
