#pragma once
#include <QDebug>

enum class InputOperation: int
{
    CLICK,
    D_CLICK,
    PRESS
};

enum class OutputState : int
{
    UNDEFINED = 0,
    LOW,
    HIGH
};

enum class LogicState : int
{
    UNDEFINED = 0,
    OFF,
    ON
};

bool outputStateToBool(OutputState state);
OutputState boolToOutputState(bool state);

bool logicStateToBool(LogicState state);
LogicState boolToLogicState(bool state);

enum class PinType : int
{
    UNUSED = 0,
    INPUT_PULLUP = 1,
    INPUT_NO_PULLUP = 2,
    OUTPUT_HIGH = 3,
    OUTPUT_LOW = 4,
    VIRTUAL_INPUT = 5,
    VIRTUAL_OUTPUT = 6,
    PWM_OUTPUT = 7
};

struct PinIdentifier
{
    PinIdentifier() {};
    PinIdentifier(const QString& idDbEntry)
    {
        auto list = idDbEntry.split(",");
        mExpanderId = list[0].toInt();
        mPinId = list[1].toInt();
    }
    PinIdentifier(int expander, int pin)
        :mExpanderId(expander), mPinId(pin)
    {
    }

    int mExpanderId;
    int mPinId;

    QString print() const
    {
        return  "expander: "+QString::number(mExpanderId)+" pin: "+QString::number(mPinId);
    }

    QString toStringDatabase() const
    {
        QString str = R"('%)";
        str.append(QString::number(mExpanderId));
        str.append(',');
        str.append(QString::number(mPinId));
        str.append(R"(%')");
        return str;
    }

    QString toStringSerial() const
    {
        QString str = QString::number(mExpanderId);
        str.append(",");
        str.append(QString::number(mPinId));
        return str;
    }

    friend bool operator== (const PinIdentifier &c1, const PinIdentifier &c2)
    {
        return ((c1.mExpanderId == c2.mExpanderId) && (c1.mPinId == c2.mPinId));
    }
    friend bool operator!= (const PinIdentifier &c1, const PinIdentifier &c2)
    {
        return ((c1.mExpanderId != c2.mExpanderId) || (c1.mPinId != c2.mPinId));
    }
    bool operator <(const PinIdentifier& rhs) const
    {
        return (mExpanderId*100)+mPinId < (rhs.mExpanderId*100)+mPinId;
    }
};
