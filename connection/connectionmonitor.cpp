#include <connection/connectionmonitor.hpp>
#include <mainapplication.hpp>

SlaveConnectionMonitor::SlaveConnectionMonitor(MainApplication *appContext)
    : mAppContext(appContext),
      mTimerThreadPool(new QThreadPool())
{
    mTimerThreadPool->setMaxThreadCount(10);
}

bool SlaveConnectionMonitor::addEntry(QString controllerName, QHostAddress controllerAddress)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    ConnectionEntry newEntry;
    newEntry.controllerAddress = controllerAddress;
    newEntry.controllerName = controllerName;

    for(auto& entry: entries)
    {
        if(entry == newEntry)
            return false;
    }

    entries.push_back(newEntry);
    return true;
}

ConnectionEntry SlaveConnectionMonitor::getKey(const QHostAddress &ipAddr)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    auto localEntries = entries;
    for(auto& entry: localEntries)
    {
        if(entry.controllerAddress == ipAddr)
        {
            entries.erase(std::remove(entries.begin(), entries.end(), entry), entries.end());
            return entry;
        }
    }
    return ConnectionEntry();
}

void SlaveConnectionMonitor::eraseEntry(const QString &controllerName)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    ConnectionEntry toErase;
    toErase.controllerName = controllerName;

    entries.erase(std::remove(entries.begin(), entries.end(), toErase), entries.end());

    qDebug() << "key " <<controllerName << " erased";
}

bool SlaveConnectionMonitor::isAvailableForSending(const QString &controllerName)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    ConnectionEntry entry;
    entry.controllerName = controllerName;

    if(std::find(entries.begin(), entries.end(), entry) != entries.end())
    {
        return false;
    }
    return true;
}

void SlaveConnectionMonitor::startTimer(const QString &controllerName)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    auto timer = new ConnectionTimer(mAppContext, controllerName);
    timer->setAutoDelete(true);
    mTimerThreadPool->start(timer);
}

