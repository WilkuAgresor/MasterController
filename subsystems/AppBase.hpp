#pragma once

#include <QObject>
#include <QRunnable>

class Components;

class AppBase: public QObject , public QRunnable
{
    Q_OBJECT

public:
    AppBase(QObject* parent, Components* components);


    enum class State
    {
        UNINITIALIZED,
        INITIALIZED,
        SHUTDOWN
    };

    void setState(State state);
    State getState() const;


    void waitUntilInitialized();

    State mState = State::INITIALIZED;
    Components* mComponents;
};
