#include "components.hpp"

Components::Components(QObject *parent)
    : QObject (parent)
    , mSender(new Sender(1234, mNetworkPortRepository, this))
    , mGrandCentral(new GrandCentral(this, this))
{
    {
        auto mainDb = DatabaseFactory::createDatabaseConnection();
        mControllers = mainDb->getControllers();
    }
    mHeatingComponents = new HeatingAppComponents(this, this);
    mLightsComponents = new LightsAppComponents(this, this);
}

void Components::sendHardwareReprovisionNotif(ControllerInfo controllerInfo)
{
    hardwareReprovisionNotif(controllerInfo);
}
