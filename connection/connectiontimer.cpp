#include <connection/connectiontimer.hpp>
#include <mainapplication.hpp>

#include <chrono>
#include <thread>

ConnectionTimer::ConnectionTimer(MainApplication *sender, QString controllerName)
    : mUdpSender(sender),
      mControllerName(controllerName)
{
}

void ConnectionTimer::run()
{
    using namespace std::chrono;

    for(int i = 50 ; i < 2000 ; i+=50)
    {
        std::this_thread::sleep_for(milliseconds(50));
    }
    mUdpSender->changeConnectionState(mControllerName, ConnectionState::TIME_OUT);
}
