#include <QString>
#include <QObject>
#include <QRunnable>
#include <subsystems/lights/LightsAppComponents.hpp>

class LightsApp : public QObject , public QRunnable
{
    Q_OBJECT
public:
    LightsApp(QObject* parent, Components* components);
    ~LightsApp() = default;

    void run();

    LightsAppComponents mComponents;
};
