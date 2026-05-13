#include <QGuiApplication>
#include <QCommandLineParser>
#include <QHostAddress>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QStandardPaths>
#include <QDir>
#include <qqml.h>

#include "database.h"
#include "reservationmodel.h"
#include "restserver.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("kde-reservas");
    QGuiApplication::setOrganizationName("Copernico");
    QGuiApplication::setApplicationDisplayName(QObject::tr("Copérnico — Reserva de Oficina"));

    QCommandLineParser parser;
    parser.setApplicationDescription("Sistema de reservas de mesa de oficina");
    parser.addHelpOption();
    QCommandLineOption portOption({"p", "port"},
                                  QGuiApplication::tr("Puerto del servicio REST."),
                                  "port", "8080");
    QCommandLineOption dbOption({"d", "database"},
                                QGuiApplication::tr("Ruta del archivo SQLite."),
                                "path");
    parser.addOption(portOption);
    parser.addOption(dbOption);
    parser.process(app);

    QQuickStyle::setStyle("Fusion");

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    const QString dbPath = parser.isSet(dbOption)
        ? parser.value(dbOption)
        : (dataDir + "/reservas.sqlite");

    Database db;
    if (!db.open(dbPath)) {
        qCritical("No se pudo abrir la base de datos en %s", qPrintable(dbPath));
        return 1;
    }
    qInfo("Base de datos: %s", qPrintable(dbPath));

    ReservationModel model(&db);

    RestServer rest(&db);
    const quint16 port = static_cast<quint16>(parser.value(portOption).toUInt());
    rest.listen(port, QHostAddress::Any);

    QObject::connect(&rest, &RestServer::reservationChanged,
                     &model, &ReservationModel::refresh,
                     Qt::QueuedConnection);

    qmlRegisterSingletonType(QUrl(QStringLiteral("qrc:/Theme.qml")),
                             "App", 1, 0, "Theme");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("reservationModel", &model);
    engine.rootContext()->setContextProperty("restPort", rest.serverPort());

    engine.load(QUrl("qrc:/main.qml"));
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }
    return app.exec();
}
