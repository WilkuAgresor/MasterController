#pragma once

#include <hardware/PinIdentifier.hpp>
#include <vector>
#include <QString>

class PinMapping
{
public:
    PinMapping(int id);

    int getId() const;

    friend bool operator== (const PinMapping &c1, const PinMapping &c2)
    {
        return c1.mId == c2.mId;
    }
    friend bool operator!= (const PinMapping &c1, const PinMapping &c2)
    {
        return c1.mId != c2.mId;
    }

    std::vector<PinIdentifier> getInputPins() const;
    std::vector<PinIdentifier> getOutputPins() const;

    void setInputPins(const QString& dbEntry);
    void setOutputPins(const QString& dbEntry);

    void addInputPin(PinIdentifier pin);
    void addOutputPin(PinIdentifier pin);

    void removeInputPin(PinIdentifier pin);
    void removeOutputPin(PinIdentifier pin);

    bool getNotifyClick();
    bool getNotifyDoubleClick();
    bool getNofityPress();

    void setNotifyClick(bool notifyClick);
    void setNotifyDoubleClick(bool notifyDoubleClick);
    void setNofityPress(bool notifyPress);

    bool containsOutputPin(const PinIdentifier& pinId) const;
    bool containsInputPin(const PinIdentifier& pinId) const;

private:

    std::vector<PinIdentifier> splitDbEntryToPinIds(const QString& entry);

    int mId;
    std::vector<PinIdentifier> mInputPins;
    std::vector<PinIdentifier> mOutputPins;
    bool mNotifyClick = false;
    bool mNotifyDoubleClick = false;
    bool mNotifyPress = false;
};
