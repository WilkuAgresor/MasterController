#include "LeoMessage.hpp"
#include <QDebug>
#include <QString>

#include "../../common/subsystems/heating/BoilerSettingsMessage.hpp"

enum class OpenthermMessageType{
    NONE,
    SET,
    GET};

class OpenthermMessage: public LeoMessage
{
public:
    OpenthermMessage(const QString& message);
    OpenthermMessage(OpenthermMessageType type);

    OpenthermMessageType getOpenthermType();
protected:
    OpenthermMessageType mOpenthermType;
};

class OpenthermSetMessage: public OpenthermMessage
{
public:
    OpenthermSetMessage(const QString& message);
    OpenthermSetMessage(const QString& name, int value);

    QString getName();
    int getValue();

private:
    QString mName;
    int mValue;
};

class OpenthermGetMessage: public OpenthermMessage
{
public:
    OpenthermGetMessage(const QString& message);
    OpenthermGetMessage();
};

BoilerSettingsPayload parseOpenthermGetResponse(const QString& message);

