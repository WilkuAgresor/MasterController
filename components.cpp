#include "components.hpp"

Components::Components(QObject *parent)
    : QObject (parent)
    , mSender(new Sender(1234, this))
{
    auto mainDb = DatabaseFactory::createDatabaseConnection();
    mControllers = mainDb->getControllers();

    mHeatingComponents = std::unique_ptr<HeatingAppComponents>(new HeatingAppComponents(this));

}
