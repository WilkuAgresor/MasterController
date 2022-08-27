

#include <thread>
#include <chrono>
#include <cstdlib>

#include <components.hpp>

#include "ScheduleApp.hpp"


ScheduleApp::ScheduleApp(QObject *parent, Components *components)
    : AppBase(parent, components)
{
}

void ScheduleApp::run()
{
    waitUntilInitialized();

    while(true)
    {        
        std::this_thread::sleep_for(std::chrono::minutes(10));
    }
}

