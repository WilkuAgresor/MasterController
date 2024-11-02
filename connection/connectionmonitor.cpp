#include <connection/connectionmonitor.hpp>
#include <mainapplication.hpp>
#include <algorithm>

SlaveConnectionMonitor::SlaveConnectionMonitor(MainApplication *appContext)
    : mAppContext(appContext),
      mTimerThreadPool(new QThreadPool())
{
    mTimerThreadPool->setMaxThreadCount(10);
}

bool SlaveConnectionMonitor::addEntry(QString controllerName, QHostAddress controllerAddress)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    ConnectionEntry newEntry(controllerName, controllerAddress);

    if(std::ranges::any_of(entries, [&newEntry](auto const& x){ return x == newEntry;}))
    {
        return false;
    }
    entries.push_back(newEntry);

    return true;
}

ConnectionEntry SlaveConnectionMonitor::getKey(const QHostAddress &ipAddr)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    if (auto result = std::ranges::find_if(entries, [&ipAddr](const auto& x){return x.controllerAddress == ipAddr;}); result !=entries.end())
    {
        auto resultCopy = *result;
        entries.erase(result);
        return resultCopy;
    }

    return ConnectionEntry();
}

void SlaveConnectionMonitor::eraseEntry(const QString &controllerName)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    std::erase(entries, controllerName);

    qDebug() << "key " <<controllerName << " erased";
}

bool SlaveConnectionMonitor::isAvailableForSending(const QString &controllerName)
{
    std::lock_guard<std::mutex> _lock(mMutex);

    if(std::ranges::any_of(entries, [&controllerName](const auto& x){return x == controllerName;}))
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

