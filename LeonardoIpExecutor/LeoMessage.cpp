#include "LeoMessage.hpp"
#include <QDebug>

LeoMessage::LeoMessage(const QString& message)
{
    mCsvData = message.split(",", QString::SplitBehavior::SkipEmptyParts);
}

LeoMessage::LeoMessage(LeoMessageType type)
{
    if(type == LeoMessageType::REPLY)
    {
        mCsvData.push_back("sts");
    }
    else if (type == LeoMessageType::RETRIEVE_REPLY )
    {
        mCsvData.push_back("rtv");
    }
    else if (type == LeoMessageType::SET_VAL )
    {
        mCsvData.push_back("setVal");
    }
    else if (type == LeoMessageType::GET_VAL )
    {
        mCsvData.push_back("getVal");
    }
}

LeoMessageType LeoMessage::getType() const
{
    auto & first = mCsvData[0];
    if(first == "sts")
    {
        return LeoMessageType::REPLY;
    }
    else if (first == "rtv")
    {
        return LeoMessageType::RETRIEVE_REPLY;
    }
    else if(first == "setVal")
    {
        return LeoMessageType::SET_VAL;
    }
    else if(first == "getVal")
    {
        return LeoMessageType::GET_VAL;
    }
    return LeoMessageType::NONE;
}

QString LeoMessage::serialize()
{
    QString message;
    for(int i=0; i < mCsvData.size(); i++)
    {
        message.append(mCsvData[i]);
        if(i != mCsvData.size()-1)
        {
            message.append(',');
        }
    }
    return message;
}

LeoSetMessage::LeoSetMessage(const QString &message)
    : LeoMessage(message)
{
    if(getType() != LeoMessageType::SET_VAL)
    {
        qWarning() << "INVALID message type for set message";
    }
    else
    {
        mExpander = std::stoi(mCsvData[1].toStdString());
        mPort = std::stoi(mCsvData[2].toStdString());
        mVal = std::stoi(mCsvData[3].toStdString());
    }
}

LeoSetMessage::LeoSetMessage(int expander, int port, int val)
    : LeoMessage(LeoMessageType::SET_VAL), mExpander(expander), mPort(port), mVal(val)
{
    mCsvData.push_back(QString::number(expander));
    mCsvData.push_back(QString::number(port));
    mCsvData.push_back(QString::number(val));
}

LeoSetMessage::LeoSetMessage(const RemotePwmSetting& setting)
    : LeoSetMessage(setting.mPin.mExpanderId, setting.mPin.mPinId, setting.mValue)
{

}

void LeoSetMessage::setValue(int val)
{
    mVal = val;
    mCsvData[3] = val;
}

LeoGetMessage::LeoGetMessage(const QString &message)
    : LeoMessage(message)
{
    if(getType() != LeoMessageType::GET_VAL)
    {
        qWarning() << "INVALID message type for set message";
    }
    else
    {
        mExpander = std::stoi(mCsvData[1].toStdString());
        mPort = std::stoi(mCsvData[2].toStdString());
    }
}

LeoGetMessage::LeoGetMessage(int expander, int port)
    : LeoMessage(LeoMessageType::GET_VAL), mExpander(expander), mPort(port)
{
    mCsvData.push_back(QString::number(expander));
    mCsvData.push_back(QString::number(port));
}


LeoReplyMessage::LeoReplyMessage(const QString &message)
    : LeoMessage(message)
{
    if(getType() != LeoMessageType::REPLY)
    {
        qWarning() << "INVALID message type for set message";
    }
    else
    {
        mStatus = std::stoi(mCsvData[1].toStdString());
    }
}

LeoReplyMessage::LeoReplyMessage(int status)
    : LeoMessage(LeoMessageType::REPLY), mStatus(status)
{
    mCsvData.push_back(QString::number(status));
}

int LeoReplyMessage::getStatus() const
{
    return mStatus;
}

LeoRetrieveReplyMessage::LeoRetrieveReplyMessage(const QString &message)
    : LeoMessage(message)
{
    if(getType() != LeoMessageType::RETRIEVE_REPLY)
    {
        qWarning() << "INVALID message type for set message";
    }
    else
    {
        mStatus = std::stoi(mCsvData[1].toStdString());
        mValue = std::stoi(mCsvData[2].toStdString());
    }
}

LeoRetrieveReplyMessage::LeoRetrieveReplyMessage(int status, int value)
    : LeoMessage(LeoMessageType::RETRIEVE_REPLY), mStatus(status), mValue(value)
{
    mCsvData.push_back(QString::number(status));
    mCsvData.push_back(QString::number(value));
}

int LeoRetrieveReplyMessage::getStatus() const
{
    return mStatus;
}

int LeoRetrieveReplyMessage::getValue() const
{
    return mValue;
}
