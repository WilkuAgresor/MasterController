#include "LeoMessage.hpp"
#include <QDebug>

LeoMessage::LeoMessage(const QString& message)
{
    mCsvData = message.split(",", QString::SplitBehavior::SkipEmptyParts);
}

LeoMessage::LeoMessage(LeoMessageType type)
{
    switch(type)
    {
    case LeoMessageType::REPLY:
            mCsvData.push_back("sts");
        break;
    case LeoMessageType::RETRIEVE_REPLY:
            mCsvData.push_back("rtv");
        break;
    case LeoMessageType::SET_VAL:
            mCsvData.push_back("setVal");
        break;
    case LeoMessageType::GET_VAL:
            mCsvData.push_back("getVal");
        break;
    case LeoMessageType::GET_SESSION_ID:
            mCsvData.push_back("getSessionId");
        break;
    case LeoMessageType::SESSION_ID_REPLY:
            mCsvData.push_back("sessionId");
        break;
    default:
        qDebug() <<"Unsupported LeoMessageType";
        break;
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
    else if(first == "getSessionId")
    {
        return LeoMessageType::GET_SESSION_ID;
    }
    else if(first == "sessionId")
    {
        return LeoMessageType::SESSION_ID_REPLY;
    }

    return LeoMessageType::NONE;
}

QByteArray LeoMessage::serialize()
{
    QString message;
    for(int i=0; i < mCsvData.size(); i++)
    {
        message.append(mCsvData[i]);
        message.append(',');
    }    
    return message.toUtf8();
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

LeoSetMessage::LeoSetMessage(const RemotePinSetting& setting)
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

LeoGetSessionIdMessage::LeoGetSessionIdMessage(const QString &message)
    : LeoMessage(message)
{
    if(getType() != LeoMessageType::GET_SESSION_ID)
    {
        qWarning() << "INVALID message type for Leo GetSessionId message";
    }
}

LeoGetSessionIdMessage::LeoGetSessionIdMessage()
    : LeoMessage(LeoMessageType::GET_SESSION_ID)
{
}

LeoSessionIdReplyMessage::LeoSessionIdReplyMessage(const QString &message)
    : LeoMessage(message)
{
    mSessionId = mCsvData[1].toUInt();
}

LeoSessionIdReplyMessage::LeoSessionIdReplyMessage(std::uint32_t sessionId)
    : LeoMessage(LeoMessageType::SESSION_ID_REPLY), mSessionId(sessionId)
{
    mCsvData.push_back(QString::number(sessionId));
}

std::uint32_t LeoSessionIdReplyMessage::getSessionId()
{
    return mSessionId;
}
