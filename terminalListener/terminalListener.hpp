#pragma once

#include <../common/receiver/receiver.hpp>
#include <components.hpp>

class TerminalListener : public Receiver
{
public:
    TerminalListener(Components* components, QObject *parent = nullptr, quint16 port = 12345);

    void handleMessage(Message msg, QHostAddress fromAddr) override;
private:
    Components* mComponents;
    std::mutex mMutex;
};
