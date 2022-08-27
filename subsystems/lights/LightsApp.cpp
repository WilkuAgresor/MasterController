#include "LightsApp.hpp"

#include <components.hpp>

#include <thread>
#include <chrono>

LightsApp::LightsApp(QObject *parent, Components *components)
    : AppBase(parent, components)
{
}

void LightsApp::run()
{
    waitUntilInitialized();

    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
//        mComponents->mGrandCentral->reprovisionOutputValues();
    }
}

