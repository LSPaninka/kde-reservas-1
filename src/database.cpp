#include "database.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

Database::Database(QObject* parent) : QObject(parent) {}

Database::~Database() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool Database::open(const QString& path) {
    QFileInfo fi(path);
    QDir().mkpath(fi.absolutePath());

    const QString connectionName = QStringLiteral("reservas-%1").arg(QUuid::createUuid().toString());
    m_db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    m_db.setDatabaseName(path);
    if (!m_db.open()) {
        qWarning("Failed to open SQLite database: %s", qPrintable(m_db.lastError().text()));
        return false;
    }
    QSqlQuery pragma(m_db);
    pragma.exec("PRAGMA foreign_keys = ON");
    pragma.exec("PRAGMA journal_mode = WAL");
    return ensureSchema();
}

bool Database::ensureSchema() {
    QSqlQuery q(m_db);
    const char* sql =
        "CREATE TABLE IF NOT EXISTS reservations ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  seat_number INTEGER NOT NULL CHECK (seat_number BETWEEN 1 AND 8),"
        "  reservation_date TEXT NOT NULL,"
        "  first_name TEXT NOT NULL,"
        "  last_name TEXT NOT NULL,"
        "  created_at TEXT NOT NULL DEFAULT (datetime('now')),"
        "  UNIQUE(seat_number, reservation_date)"
        ")";
    if (!q.exec(sql)) {
        qWarning("Schema creation failed: %s", qPrintable(q.lastError().text()));
        return false;
    }
    q.exec("CREATE INDEX IF NOT EXISTS idx_reservations_date ON reservations(reservation_date)");
    return true;
}

QVariantList Database::seatsForDate(const QString& isoDate) {
    QVariantList result;
    QSqlQuery q(m_db);
    q.prepare("SELECT seat_number, first_name, last_name, id "
              "FROM reservations WHERE reservation_date = :d");
    q.bindValue(":d", isoDate);

    QVariantMap byseat;
    if (q.exec()) {
        while (q.next()) {
            QVariantMap row;
            const int seat = q.value(0).toInt();
            row["seat"] = seat;
            row["reserved"] = true;
            row["first_name"] = q.value(1).toString();
            row["last_name"] = q.value(2).toString();
            row["id"] = q.value(3).toInt();
            byseat.insert(QString::number(seat), row);
        }
    } else {
        qWarning("seatsForDate query failed: %s", qPrintable(q.lastError().text()));
    }

    for (int s = 1; s <= SEATS_PER_TABLE; ++s) {
        const QString key = QString::number(s);
        if (byseat.contains(key)) {
            result.append(byseat.value(key));
        } else {
            QVariantMap empty;
            empty["seat"] = s;
            empty["reserved"] = false;
            empty["first_name"] = QString();
            empty["last_name"] = QString();
            empty["id"] = 0;
            result.append(empty);
        }
    }
    return result;
}

bool Database::reserveSeat(int seat, const QString& isoDate,
                           const QString& firstName, const QString& lastName,
                           QString* error) {
    if (seat < 1 || seat > SEATS_PER_TABLE) {
        if (error) *error = "seat must be between 1 and 8";
        return false;
    }
    if (firstName.trimmed().isEmpty() || lastName.trimmed().isEmpty()) {
        if (error) *error = "first_name and last_name are required";
        return false;
    }
    if (isoDate.isEmpty()) {
        if (error) *error = "date is required (YYYY-MM-DD)";
        return false;
    }

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO reservations(seat_number, reservation_date, first_name, last_name) "
              "VALUES (:s, :d, :fn, :ln)");
    q.bindValue(":s", seat);
    q.bindValue(":d", isoDate);
    q.bindValue(":fn", firstName.trimmed());
    q.bindValue(":ln", lastName.trimmed());
    if (!q.exec()) {
        const QString err = q.lastError().text();
        if (error) {
            if (err.contains("UNIQUE", Qt::CaseInsensitive)) {
                *error = "seat already reserved for that date";
            } else {
                *error = err;
            }
        }
        return false;
    }
    return true;
}

bool Database::cancelReservation(int seat, const QString& isoDate, QString* error) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM reservations WHERE seat_number = :s AND reservation_date = :d");
    q.bindValue(":s", seat);
    q.bindValue(":d", isoDate);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() <= 0) {
        if (error) *error = "no reservation found for that seat and date";
        return false;
    }
    return true;
}
