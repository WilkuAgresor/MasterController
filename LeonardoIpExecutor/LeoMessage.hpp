#pragma once
#include <QString>
#include <QStringList>
#include "RemoteRGBSetting.hpp"

enum class LeoMessageType{
    NONE,
    REPLY,
    RETRIEVE_REPLY,
    SET_VAL,
    GET_VAL,
    GET_SESSION_ID,
    SESSION_ID_REPLY
};

class LeoMessage
{
public:
    LeoMessage(const QString& message);
    LeoMessage(LeoMessageType type);

    LeoMessageType getType() const;

    QByteArray serialize();

    QStringList mCsvData;
};


class LeoSetMessage : public LeoMessage
{
public:
    LeoSetMessage(const QString& message);
    LeoSetMessage(int expander, int port, int val);
    LeoSetMessage(const RemotePinSetting &setting);

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

// --------------- SESSION ID MESSAGE

class LeoGetSessionIdMessage: public LeoMessage
{
public:
    LeoGetSessionIdMessage(const QString& message);
    LeoGetSessionIdMessage();
};

class LeoSessionIdReplyMessage: public LeoMessage
{
public:
    LeoSessionIdReplyMessage(const QString& message);
    LeoSessionIdReplyMessage(std::uint32_t sessionId);

    std::uint32_t getSessionId();
private:
    std::uint32_t mSessionId = 0;
};
