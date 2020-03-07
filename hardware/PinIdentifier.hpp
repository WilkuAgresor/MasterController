#pragma once
#include <QDebug>

enum class InputOperation: int
{
    CLICK,
    D_CLICK,
    PRESS
};

struct PinIdentifier
{
    int mExpanderId;
    int mPinId;

    void print()
    {
        qDebug() << "expander: "<<mExpanderId<<" pin: "<<mPinId;
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
