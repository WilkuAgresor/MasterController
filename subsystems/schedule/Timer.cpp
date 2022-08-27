#include "Timer.hpp"
#include <QDebug>
#include <QDateTime>

MyTimer::MyTimer()
{

}

void MyTimer::scheduleNext(const SchedEvent &event)
{
    if(timer && timer->isActive())
    {
        timer->stop();
    }

    // create a timer
    timer = new QTimer(this);
    timer->setSingleShot(true);
    // setup signal and slot
    connect(timer, SIGNAL(timeout()),
          this, SLOT(MyTimerSlot()));

    auto timestamp = QDateTime::fromString(event.mTrigger);

    auto now = QDateTime::currentDateTime();

    auto timerDelay = timestamp.toMSecsSinceEpoch() - now.toMSecsSinceEpoch();

    // msec
    timer->start(timerDelay);
}

void MyTimer::MyTimerSlot()
{
    qDebug() << "Timer...";
}
