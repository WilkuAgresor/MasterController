#include "terminalListener.hpp"
#include "messageHandler.hpp"

TerminalListener::TerminalListener(Components* components, QObject *parent, quint16 port)
    : Receiver(port, components->mNetworkPortRepository, parent),
      mComponents(components)
{

}

void TerminalListener::handleMessage(Message msg, QHostAddress fromAddr)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    try
    {
        auto messageHandler = new MessageHandler(this, std::move(msg), fromAddr, mComponents);
        messageHandler->setAutoDelete(true);
        QThreadPool::globalInstance()->start(messageHandler);
    }
    catch (const std::exception& ex)
    {
        qDebug() << ex.what();
    }

}
