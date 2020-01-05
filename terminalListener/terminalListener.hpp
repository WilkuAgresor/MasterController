#pragma once

#include <../common/receiver/receiver.hpp>
#include <components.hpp>

class TerminalListener : public Receiver
{
public:
    TerminalListener(Sender* sender, Components* components, QObject *parent = nullptr, quint16 port = 12345);

    void handleMessage(QNetworkDatagram msg) override;
private:
    Components* mComponents;
    std::mutex mMutex;
};
