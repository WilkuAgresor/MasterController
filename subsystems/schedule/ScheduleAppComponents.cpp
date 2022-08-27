#include "ScheduleAppComponents.hpp"
#include <components.hpp>

ScheduleAppComponents::ScheduleAppComponents(Components *components)
    : mSystemComponents(components)
{

}

void ScheduleAppComponents::retrieveAllData(QHostAddress terminalAddr, quint16 port)
{
    SchedPayload schedPayload;

    std::vector<SchedProfile> profiles;
    std::vector<SchedEventTyp> types;
    std::vector<SchedEvent> events;

    schedPayload.mTypes = types;
    schedPayload.mProfiles = profiles;
    schedPayload.mEvents = events;


    SchedMessage message(schedPayload);
    qDebug() <<"schedule reprovision settings: " << schedPayload.toString();

    mSystemComponents->mSender->send(terminalAddr, port, message.toData());
}

void ScheduleAppComponents::reprovisionTerminalData(QHostAddress terminalAddr)
{
    return retrieveAllData(terminalAddr, TERMINAL_LISTEN_PORT);
}

void ScheduleAppComponents::handleMessage(const Message &message, QHostAddress fromAddr)
try
{
    auto& schedMessage = static_cast<const SchedMessage&>(message);

    auto payload = schedMessage.payload();

    switch(payload.mOperation)
    {
    case ScheduleOperationType::RETRIEVE:
        qDebug() << "Sched Retrieve";
        retrieveAllData(fromAddr, message.getHeader().mReplyPort);
        break;
    case ScheduleOperationType::ADD:
        qDebug() << "Sched Add";
        break;
    case ScheduleOperationType::REMOVE:
        qDebug() << "Sched Remove";
        break;
    }
}
catch(const std::exception& ex)
{
    qWarning() << "Exception thrown during Schedule message handling: "<<ex.what();
}
