#include "PinMapping.hpp"
#include <QStringList>
#include <QDebug>

PinMapping::PinMapping(int id)
    : mId(id)
{

}

int PinMapping::getId() const
{
    return mId;
}

std::vector<PinIdentifier> PinMapping::getInputPins() const
{
    return mInputPins;
}

std::vector<PinIdentifier> PinMapping::getOutputPins() const
{
    return mOutputPins;
}

void PinMapping::setInputPins(const QString &dbEntry)
{
    mInputPins = splitDbEntryToPinIds(dbEntry);
}

void PinMapping::setOutputPins(const QString &dbEntry)
{
    mOutputPins = splitDbEntryToPinIds(dbEntry);
}

bool PinMapping::getNotifyClick()
{
    return mNotifyClick;
}

bool PinMapping::getNotifyDoubleClick()
{
    return mNotifyDoubleClick;
}

bool PinMapping::getNofityPress()
{
    return mNotifyPress;
}

void PinMapping::setNotifyClick(bool notifyClick)
{
    mNotifyClick = notifyClick;
}

void PinMapping::setNotifyDoubleClick(bool notifyDoubleClick)
{
    mNotifyDoubleClick = notifyDoubleClick;
}

void PinMapping::setNofityPress(bool notifyPress)
{
    mNotifyPress = notifyPress;
}

bool PinMapping::containsOutputPin(const PinIdentifier &pinId) const
{
    return std::find(mOutputPins.begin(), mOutputPins.end(), pinId) != mOutputPins.end();
}

bool PinMapping::containsInputPin(const PinIdentifier &pinId) const
{
    return std::find(mInputPins.begin(), mInputPins.end(), pinId) != mInputPins.end();
}

std::vector<PinIdentifier> PinMapping::splitDbEntryToPinIds(const QString &entry)
{
    //single id -  x,y
    //id separator - '**'
    //full entry x1,y1**x2,y2**x3,y3

    std::vector<PinIdentifier> pinIds;

    auto entries = entry.split("**", Qt::SkipEmptyParts);

    std::ranges::transform(entries, std::back_inserter(pinIds), [](const auto& x){
                                                                    PinIdentifier id;
                                                                    auto values = x.split(",");
                                                                    id.mExpanderId = values[0].toInt();
                                                                    id.mPinId = values[1].toInt();
                                                                    return id;});

    return pinIds;
}
