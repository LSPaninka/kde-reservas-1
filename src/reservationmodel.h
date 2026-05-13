#pragma once

#include <QAbstractListModel>
#include <QDate>
#include <QVariantList>

class Database;

class ReservationModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString currentDate READ currentDate WRITE setCurrentDate NOTIFY currentDateChanged)
    Q_PROPERTY(QString displayDate READ displayDate NOTIFY currentDateChanged)
    Q_PROPERTY(int seatCount READ seatCount CONSTANT)

public:
    enum Roles {
        SeatRole = Qt::UserRole + 1,
        ReservedRole,
        FirstNameRole,
        LastNameRole,
        FullNameRole,
        IdRole,
    };

    explicit ReservationModel(Database* db, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentDate() const { return m_currentDate; }
    void setCurrentDate(const QString& isoDate);

    QString displayDate() const;
    int seatCount() const;

public slots:
    void refresh();
    bool reserve(int seat, const QString& firstName, const QString& lastName);
    bool cancel(int seat);
    QString lastError() const { return m_lastError; }
    void goToToday();

signals:
    void currentDateChanged();
    void errorOccurred(const QString& message);

private:
    Database* m_db;
    QString m_currentDate;
    QVariantList m_rows;
    mutable QString m_lastError;
};
