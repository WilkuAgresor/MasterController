#include <QString>
#include <QObject>
#include <components.hpp>
#include <../common/subsystems/AppBase.hpp>

class TopologyApp : public AppBase
{
    Q_OBJECT
public:
    TopologyApp(QObject* parent, Components *components);
    ~TopologyApp();

    void run();

    int i = 0;

signals:
    void hardwareReprovisionNotif(ControllerInfo controllerInfo);

};
