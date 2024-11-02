#include "OpenthermMessages.hpp"


OpenthermMessage::OpenthermMessage(const QString &message)
    : LeoMessage(message)
{
    mOpenthermType = getOpenthermType();
}

OpenthermMessage::OpenthermMessage(OpenthermMessageType type)
    : LeoMessage(LeoMessageType::OPENTHERM)
    , mOpenthermType(type)
{
    switch(type)
    {
    case OpenthermMessageType::SET:
        mCsvData.push_back("set");
        break;
    case OpenthermMessageType::GET:
        mCsvData.push_back("get");
        break;
    default:
        qWarning() << "Unsupported opentherm message type";
        break;
    }
}

OpenthermMessageType OpenthermMessage::getOpenthermType()
{
    auto & typeStr = mCsvData[1];
    if(typeStr == "set")
    {
        return OpenthermMessageType::SET;
    }
    else if (typeStr == "get")
    {
        return OpenthermMessageType::GET;
    }
    return OpenthermMessageType::NONE;
}

OpenthermSetMessage::OpenthermSetMessage(const QString &message)
    : OpenthermMessage(message)
{
    if(mCsvData.size() < 4)
    {
        qWarning() << "invalid opentherm,set message";
    }
    else
    {
        mName = mCsvData[2];
        mValue = mCsvData[3].toInt();
    }
}

OpenthermSetMessage::OpenthermSetMessage(const QString &name, int value)
    : OpenthermMessage(OpenthermMessageType::SET)
    , mName(name)
    , mValue(value)
{
    mCsvData = QStringList { name, QString::number(value) };
}

QString OpenthermSetMessage::getName()
{
    return mName;
}

int OpenthermSetMessage::getValue()
{
    return mValue;
}

OpenthermGetMessage::OpenthermGetMessage(const QString &message)
    : OpenthermMessage(message)
{
}

OpenthermGetMessage::OpenthermGetMessage()
    : OpenthermMessage(OpenthermMessageType::GET)
{
}

BoilerSettingsPayload parseOpenthermGetResponse(const QString &message)
{
    BoilerSettingsPayload settings;

    auto segments = message.split(",", Qt::SkipEmptyParts);

    for(int i = 0; i < segments.size() - 1; i++)
    {
        auto name = segments[i];
        auto value = segments[i+1].toFloat();

        // Read-only properties
        if (name == JSON_IS_CENTRAL_HEATING_ON)
            settings.mIsCentralHeatingOn = value == 1;
        if (name == JSON_IS_HOT_WATER_ON)
            settings.mIsHotWaterOn = value == 1;
        if (name == JSON_IS_FLAME_ON)
            settings.mIsFlameOn = value == 1;
        if (name == JSON_CENTRAL_HEATING_TEMP)
            settings.mCentralHeatingTemp = value;
        if (name == JSON_MAX_CENTRAL_HEATING_TEMP)
            settings.mMaxCentralHeatingTemp = value;
        if (name == JSON_HOT_WATER_TEMP)
            settings.mHotWaterTemp = value;
        if (name == JSON_CONNECTION_STATUS)
            settings.mConnectionStatus = value;
        if (name == JSON_RETURN_TEMP)
            settings.mReturnTemp = value;
        if (name == JSON_PRESSURE)
            settings.mPressure = value * 100;
    }
    qDebug() <<"parsed get response: " << settings.toString();

    return settings;
}
