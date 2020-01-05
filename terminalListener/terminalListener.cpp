#include "terminalListener.hpp"
#include "messageHandler.hpp"

TerminalListener::TerminalListener(Sender */*sender*/, Components* components, QObject *parent, quint16 port)
    : Receiver(port, parent),
      mComponents(components)
{

}

void TerminalListener::handleMessage(QNetworkDatagram msg)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    try
    {
        auto messageHandler = new MessageHandler(msg, mComponents);
        messageHandler->setAutoDelete(true);
        connect(messageHandler, SIGNAL(result(QNetworkDatagram)), this, SLOT(sendResponse(QNetworkDatagram)),Qt::QueuedConnection);
        QThreadPool::globalInstance()->start(messageHandler);
    }
    catch (const std::exception& ex)
    {
        qDebug() << ex.what();
    }

}
