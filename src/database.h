#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class Database : public QObject {
    Q_OBJECT
public:
    static constexpr int SEATS_PER_TABLE = 8;

    explicit Database(QObject* parent = nullptr);
    ~Database() override;

    bool open(const QString& path);

    // Returns a list of 8 entries (seat 1..8). Each entry is a QVariantMap with:
    //   seat (int), reserved (bool), first_name (QString), last_name (QString), id (int)
    QVariantList seatsForDate(const QString& isoDate);

    // Reserves a seat. Returns true on success; on failure sets *error.
    bool reserveSeat(int seat, const QString& isoDate,
                     const QString& firstName, const QString& lastName,
                     QString* error = nullptr);

    // Cancels a reservation. Returns true on success.
    bool cancelReservation(int seat, const QString& isoDate, QString* error = nullptr);

private:
    bool ensureSchema();

    QSqlDatabase m_db;
};
