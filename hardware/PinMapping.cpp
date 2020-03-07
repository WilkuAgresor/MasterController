#include "PinMapping.hpp"
#include <QStringList>
#include <QDebug>

PinMapping::PinMapping(int id)
    : mId(id)
{

}

int PinMapping::getId()
{
    return mId;
}

std::vector<PinIdentifier> PinMapping::getInputPins()
{
    return mInputPins;
}

std::vector<PinIdentifier> PinMapping::getOutputPins()
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

std::vector<PinIdentifier> PinMapping::splitDbEntryToPinIds(const QString &entry)
{
    //single id -  x,y
    //id separator - '**'
    //full entry x1,y1**x2,y2**x3,y3

    std::vector<PinIdentifier> pinIds;

    auto entries = entry.split("**", QString::SkipEmptyParts);

    for(auto& entry: entries)
    {
        qDebug() << "entry: "<<entry;
        PinIdentifier id;
        auto values = entry.split(",");
        id.mExpanderId = values[0].toInt();
        id.mPinId = values[1].toInt();
        pinIds.push_back(id);
    }

    return pinIds;
}
