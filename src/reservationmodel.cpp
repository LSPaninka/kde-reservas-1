#include "reservationmodel.h"

#include "database.h"

#include <QDate>
#include <QLocale>

ReservationModel::ReservationModel(Database* db, QObject* parent)
    : QAbstractListModel(parent), m_db(db) {
    m_currentDate = QDate::currentDate().toString(Qt::ISODate);
    refresh();
}

int ReservationModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_rows.size();
}

int ReservationModel::seatCount() const {
    return Database::SEATS_PER_TABLE;
}

QVariant ReservationModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
        return {};
    }
    const QVariantMap row = m_rows.at(index.row()).toMap();
    switch (role) {
        case SeatRole:      return row.value("seat");
        case ReservedRole:  return row.value("reserved");
        case FirstNameRole: return row.value("first_name");
        case LastNameRole:  return row.value("last_name");
        case IdRole:        return row.value("id");
        case FullNameRole: {
            const QString fn = row.value("first_name").toString();
            const QString ln = row.value("last_name").toString();
            if (fn.isEmpty() && ln.isEmpty()) return QString();
            return QStringLiteral("%1 %2").arg(fn, ln).trimmed();
        }
        default: return {};
    }
}

QHash<int, QByteArray> ReservationModel::roleNames() const {
    return {
        { SeatRole,      "seat" },
        { ReservedRole,  "reserved" },
        { FirstNameRole, "firstName" },
        { LastNameRole,  "lastName" },
        { FullNameRole,  "fullName" },
        { IdRole,        "reservationId" },
    };
}

void ReservationModel::setCurrentDate(const QString& isoDate) {
    if (isoDate == m_currentDate) return;
    QDate d = QDate::fromString(isoDate, Qt::ISODate);
    if (!d.isValid()) return;
    m_currentDate = d.toString(Qt::ISODate);
    emit currentDateChanged();
    refresh();
}

QString ReservationModel::displayDate() const {
    const QDate d = QDate::fromString(m_currentDate, Qt::ISODate);
    if (!d.isValid()) return m_currentDate;
    const QLocale es(QLocale::Spanish);
    const QString dayShort = es.dayName(d.dayOfWeek(), QLocale::ShortFormat);
    QString s = dayShort;
    if (!s.isEmpty()) s[0] = s[0].toUpper();
    return QStringLiteral("%1 %2/%3/%4")
        .arg(s)
        .arg(d.day())
        .arg(d.month())
        .arg(d.year() % 100, 2, 10, QChar('0'));
}

void ReservationModel::refresh() {
    beginResetModel();
    m_rows = m_db->seatsForDate(m_currentDate);
    endResetModel();
}

bool ReservationModel::reserve(int seat, const QString& firstName, const QString& lastName) {
    QString err;
    if (!m_db->reserveSeat(seat, m_currentDate, firstName, lastName, &err)) {
        m_lastError = err;
        emit errorOccurred(err);
        return false;
    }
    m_lastError.clear();
    refresh();
    return true;
}

bool ReservationModel::cancel(int seat) {
    QString err;
    if (!m_db->cancelReservation(seat, m_currentDate, &err)) {
        m_lastError = err;
        emit errorOccurred(err);
        return false;
    }
    m_lastError.clear();
    refresh();
    return true;
}

void ReservationModel::goToToday() {
    setCurrentDate(QDate::currentDate().toString(Qt::ISODate));
}
