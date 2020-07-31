#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>
#include <QtDebug>
#include <QTextStream>
#include <mainapplication.hpp>
#include <iostream>
#include <QDateTime>
#include <QFile>

//static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(0);

void myMessageOutput(QtMsgType type, const QMessageLogContext &/*context*/, const QString &msg)
try
{        
    QString message = msg;
    QString finalLogMessage = QDateTime::currentDateTime().toString(Qt::ISODateWithMs)+ " | " + message;

    static QMutex mutex;
    QMutexLocker lock(&mutex);

    static QFile errorLog("/opt/log/error.log");
    static QFile debugLog("/opt/log/debug.log");
    static bool errorLogIsOpen = errorLog.open(QIODevice::Append | QIODevice::Text);
    static bool debugLogIsOpen = debugLog.open(QIODevice::Append | QIODevice::Text);


    if (type == QtMsgType::QtDebugMsg && debugLogIsOpen)
    {
        QTextStream out(&debugLog);
        out << finalLogMessage << '\n';
    }
    else if(errorLogIsOpen)
    {
        QTextStream out(&errorLog);
        out << finalLogMessage << '\n';
    }



//    QString message = msg;
//    static std::mutex sLoggerMutex;
//    static QFile errorLog("/opt/log/error.log");
//    static QFile debugLog("/opt/log/debug.log");

//    std::cout << "log type: "<<type;

//    std::lock_guard<std::mutex> _lock(sLoggerMutex);
//    if (!debugLog.isOpen() && !debugLog.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append))
//    {
//        std::cout << "failed to open log file";
//        return;
//    }

//    if (!errorLog.isOpen() && !errorLog.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append))
//    {
//        std::cout << "failed to open log file";
//        return;
//    }

//    static QTextStream debugOut(&debugLog);
//    static QTextStream errorOut(&errorLog);

//    if(type == QtMsgType::QtDebugMsg)
//    {
//        debugOut << finalLogMessage << '\n';
//    }
//    else if(type == QtMsgType::QtWarningMsg || type ==QtMsgType::QtCriticalMsg)
//    {
//        errorOut << finalLogMessage << '\n';
//    }
// //   (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, finalLogMessage);
}
catch(const std::exception&)
{
    //nothing to do. it's just a debug log
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    qRegisterMetaType<QNetworkDatagram>("QNetworkDatagram");

    QGuiApplication app(argc, argv);
    qInstallMessageHandler(myMessageOutput);
    qWarning() << "CONTROLLER START";

    QScopedPointer<MainApplication> mainApp(new MainApplication(&app));

    return app.exec();
}
