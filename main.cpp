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

static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(0);

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString finalLogMessage = QDateTime::currentDateTime().toString(Qt::ISODateWithMs)+ " | " + msg;

    (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, finalLogMessage);
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    qRegisterMetaType<QNetworkDatagram>("QNetworkDatagram");

    QGuiApplication app(argc, argv);
    qInstallMessageHandler(myMessageOutput);

    QScopedPointer<MainApplication> mainApp(new MainApplication(&app));

    return app.exec();
}
