#pragma once
#include <QString>
#include <QStringList>
#include "RemoteRGBSetting.hpp"

enum class LeoMessageType{
    NONE,
    REPLY,
    RETRIEVE_REPLY,
    SET_VAL,
    GET_VAL
};

class LeoMessage
{
public:
    LeoMessage(const QString& message);
    LeoMessage(LeoMessageType type);

    LeoMessageType getType() const;

    QString serialize();

    QStringList mCsvData;
};


class LeoSetMessage : public LeoMessage
{
public:
    LeoSetMessage(const QString& message);
    LeoSetMessage(int expander, int port, int val);
    LeoSetMessage(const RemotePwmSetting &setting);

    void setValue(int val);

private:
    int mExpander = -1;
    int mPort = -1;
    int mVal = -1;
};


class LeoGetMessage : public LeoMessage
{
public:
    LeoGetMessage(const QString& message);
    LeoGetMessage(int expander, int port);

private:
    int mExpander = -1;
    int mPort = -1;
};

class LeoReplyMessage : public LeoMessage
{
public:
    LeoReplyMessage(const QString& message);
    LeoReplyMessage(int status);

    int getStatus() const;

private:
    int mStatus = -1;
};


class LeoRetrieveReplyMessage : public LeoMessage
{
public:
    LeoRetrieveReplyMessage(const QString& message);
    LeoRetrieveReplyMessage(int status, int value);

    int getStatus() const;
    int getValue() const;

private:
    int mStatus = -1;
    int mValue = -1;
};
