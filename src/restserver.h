#pragma once

#include <QObject>
#include <QTcpServer>

class Database;
class QTcpSocket;

class RestServer : public QObject {
    Q_OBJECT
public:
    explicit RestServer(Database* db, QObject* parent = nullptr);

    bool listen(quint16 port = 8080, const QHostAddress& address = QHostAddress::LocalHost);
    quint16 serverPort() const { return m_server.serverPort(); }

signals:
    void reservationChanged();

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    void handleRequest(QTcpSocket* socket, const QByteArray& request);
    void writeResponse(QTcpSocket* socket, int code, const QString& status,
                       const QByteArray& body, const QByteArray& contentType = "application/json");
    void writeJsonError(QTcpSocket* socket, int code, const QString& status, const QString& message);

    Database* m_db;
    QTcpServer m_server;
};
