#include "LightsApp.hpp"

#include <components.hpp>

#include <thread>
#include <chrono>

LightsApp::LightsApp(QObject *parent, Components *components)
    : QObject (parent)
    , mComponents(components)
{
}

void LightsApp::run()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
//        mComponents->mGrandCentral->reprovisionOutputValues();
    }
}

