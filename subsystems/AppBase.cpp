#include "AppBase.hpp"
#include <thread>
#include <chrono>

AppBase::AppBase(QObject *parent, Components *components)
    : QObject (parent), mComponents(components)
{

}

void AppBase::setState(AppBase::State state)
{
    mState = state;
}

AppBase::State AppBase::getState() const
{
    return mState;
}

void AppBase::waitUntilInitialized()
{
    while(true)
    {
        if(mState != State::UNINITIALIZED)
        {
            break;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}
