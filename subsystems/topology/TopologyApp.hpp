#include <QString>
#include <QObject>
#include <components.hpp>



class TopologyApp : public QObject , public QRunnable
{
    Q_OBJECT
public:
    TopologyApp(QObject* parent, Components& components);
    ~TopologyApp() = default;

    void run();

    int i = 0;

    Components& mComponents;
};
