#include "restserver.h"

#include "database.h"

#include <QDate>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>

RestServer::RestServer(Database* db, QObject* parent)
    : QObject(parent), m_db(db) {
    connect(&m_server, &QTcpServer::newConnection, this, &RestServer::onNewConnection);
}

bool RestServer::listen(quint16 port, const QHostAddress& address) {
    if (!m_server.listen(address, port)) {
        qWarning("REST server failed to listen on %s:%u: %s",
                 qPrintable(address.toString()), port,
                 qPrintable(m_server.errorString()));
        return false;
    }
    qInfo("REST server listening on %s:%u",
          qPrintable(address.toString()), m_server.serverPort());
    return true;
}

void RestServer::onNewConnection() {
    while (m_server.hasPendingConnections()) {
        QTcpSocket* socket = m_server.nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &RestServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    }
}

static QByteArray readBody(const QByteArray& request, int headerEnd) {
    // Expect Content-Length header. Body starts after headerEnd + 4 (\r\n\r\n).
    const QByteArray headers = request.left(headerEnd);
    int contentLength = 0;
    for (const QByteArray& line : headers.split('\n')) {
        const QByteArray trimmed = line.trimmed();
        if (trimmed.toLower().startsWith("content-length:")) {
            contentLength = trimmed.mid(QByteArray("content-length:").size()).trimmed().toInt();
            break;
        }
    }
    QByteArray body = request.mid(headerEnd + 4);
    if (contentLength > 0 && body.size() > contentLength) {
        body = body.left(contentLength);
    }
    return body;
}

void RestServer::onReadyRead() {
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray buffer = socket->property("buffer").toByteArray();
    buffer.append(socket->readAll());

    int headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd < 0) {
        socket->setProperty("buffer", buffer);
        return;
    }
    // Wait until full body is received.
    int contentLength = 0;
    for (const QByteArray& line : buffer.left(headerEnd).split('\n')) {
        const QByteArray trimmed = line.trimmed();
        if (trimmed.toLower().startsWith("content-length:")) {
            contentLength = trimmed.mid(QByteArray("content-length:").size()).trimmed().toInt();
            break;
        }
    }
    const int needed = headerEnd + 4 + contentLength;
    if (buffer.size() < needed) {
        socket->setProperty("buffer", buffer);
        return;
    }

    socket->setProperty("buffer", QByteArray());
    handleRequest(socket, buffer.left(needed));
}

void RestServer::handleRequest(QTcpSocket* socket, const QByteArray& request) {
    const int headerEnd = request.indexOf("\r\n\r\n");
    const QByteArray headerBlock = request.left(headerEnd);
    const int firstLineEnd = headerBlock.indexOf("\r\n");
    const QByteArray requestLine = firstLineEnd >= 0 ? headerBlock.left(firstLineEnd) : headerBlock;
    const QList<QByteArray> parts = requestLine.split(' ');
    if (parts.size() < 3) {
        writeJsonError(socket, 400, "Bad Request", "malformed request line");
        socket->disconnectFromHost();
        return;
    }
    const QByteArray method = parts.at(0);
    const QUrl url = QUrl::fromEncoded(parts.at(1));
    const QString path = url.path();
    const QUrlQuery query(url);

    if (method == "GET" && (path == "/health" || path == "/")) {
        QJsonObject o{{"status", "ok"}, {"seats_per_table", Database::SEATS_PER_TABLE}};
        writeResponse(socket, 200, "OK", QJsonDocument(o).toJson(QJsonDocument::Compact));
        socket->disconnectFromHost();
        return;
    }

    if (method == "GET" && (path == "/seats" || path == "/reservations")) {
        QString isoDate = query.queryItemValue("date");
        if (isoDate.isEmpty()) {
            isoDate = QDate::currentDate().toString(Qt::ISODate);
        }
        const QDate d = QDate::fromString(isoDate, Qt::ISODate);
        if (!d.isValid()) {
            writeJsonError(socket, 400, "Bad Request", "invalid date; expected YYYY-MM-DD");
            socket->disconnectFromHost();
            return;
        }
        const QVariantList rows = m_db->seatsForDate(isoDate);
        QJsonArray arr;
        for (const QVariant& v : rows) {
            const QVariantMap m = v.toMap();
            QJsonObject o;
            o["seat"] = m.value("seat").toInt();
            o["reserved"] = m.value("reserved").toBool();
            o["first_name"] = m.value("first_name").toString();
            o["last_name"] = m.value("last_name").toString();
            o["id"] = m.value("id").toInt();
            arr.append(o);
        }
        QJsonObject resp{{"date", isoDate}, {"seats", arr}};
        writeResponse(socket, 200, "OK", QJsonDocument(resp).toJson(QJsonDocument::Compact));
        socket->disconnectFromHost();
        return;
    }

    if (method == "POST" && (path == "/seats" || path == "/reservations")) {
        const QByteArray body = readBody(request, headerEnd);
        QJsonParseError perr;
        const QJsonDocument doc = QJsonDocument::fromJson(body, &perr);
        if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
            writeJsonError(socket, 400, "Bad Request", "invalid JSON body");
            socket->disconnectFromHost();
            return;
        }
        const QJsonObject obj = doc.object();
        const int seat = obj.value("seat").toInt(-1);
        QString isoDate = obj.value("date").toString();
        if (isoDate.isEmpty()) isoDate = QDate::currentDate().toString(Qt::ISODate);
        const QString fn = obj.value("first_name").toString();
        const QString ln = obj.value("last_name").toString();

        const QDate d = QDate::fromString(isoDate, Qt::ISODate);
        if (!d.isValid()) {
            writeJsonError(socket, 400, "Bad Request", "invalid date; expected YYYY-MM-DD");
            socket->disconnectFromHost();
            return;
        }

        QString err;
        if (!m_db->reserveSeat(seat, isoDate, fn, ln, &err)) {
            const int code = err.contains("already reserved") ? 409 : 400;
            writeJsonError(socket, code, code == 409 ? "Conflict" : "Bad Request", err);
            socket->disconnectFromHost();
            return;
        }

        QJsonObject resp{
            {"seat", seat},
            {"date", isoDate},
            {"first_name", fn.trimmed()},
            {"last_name", ln.trimmed()},
            {"reserved", true},
        };
        writeResponse(socket, 201, "Created", QJsonDocument(resp).toJson(QJsonDocument::Compact));
        emit reservationChanged();
        socket->disconnectFromHost();
        return;
    }

    if (method == "DELETE" && (path == "/seats" || path == "/reservations")) {
        const int seat = query.queryItemValue("seat").toInt();
        QString isoDate = query.queryItemValue("date");
        if (isoDate.isEmpty()) isoDate = QDate::currentDate().toString(Qt::ISODate);
        QString err;
        if (!m_db->cancelReservation(seat, isoDate, &err)) {
            writeJsonError(socket, 404, "Not Found", err);
            socket->disconnectFromHost();
            return;
        }
        writeResponse(socket, 200, "OK", "{\"ok\":true}");
        emit reservationChanged();
        socket->disconnectFromHost();
        return;
    }

    writeJsonError(socket, 404, "Not Found", "unknown endpoint");
    socket->disconnectFromHost();
}

void RestServer::writeResponse(QTcpSocket* socket, int code, const QString& status,
                               const QByteArray& body, const QByteArray& contentType) {
    QByteArray response;
    response.append(QString("HTTP/1.1 %1 %2\r\n").arg(code).arg(status).toUtf8());
    response.append("Content-Type: " + contentType + "\r\n");
    response.append("Content-Length: " + QByteArray::number(body.size()) + "\r\n");
    response.append("Access-Control-Allow-Origin: *\r\n");
    response.append("Connection: close\r\n\r\n");
    response.append(body);
    socket->write(response);
    socket->flush();
}

void RestServer::writeJsonError(QTcpSocket* socket, int code, const QString& status, const QString& message) {
    QJsonObject o{{"error", message}};
    writeResponse(socket, code, status, QJsonDocument(o).toJson(QJsonDocument::Compact));
}
