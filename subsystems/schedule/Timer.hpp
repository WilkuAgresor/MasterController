#include <QTimer>
#include <../common/subsystems/schedule/ScheduleEventTypes.hpp>
class MyTimer : public QObject
{
    Q_OBJECT
public:
    MyTimer();
    QTimer *timer;

    void scheduleNext(const SchedEvent& event);

public slots:
    void MyTimerSlot();
private:
    SchedEvent currentEvent;
};
