#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QString>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager &instance();

    bool open(const QString &databasePath);
    void close();
    bool isOpen() const;
    QSqlDatabase database() const;
    QString databasePath() const;
    QString errorString() const;

    bool authenticate(const QString &username,
                      const QString &password,
                      QString *userId = nullptr,
                      QString *fullName = nullptr,
                      QString *error = nullptr);
    bool logEvent(const QString &userId,
                  const QString &event,
                  QString *error = nullptr);

    static QString hashPassword(const QString &password);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    bool createSchema();
    bool seedData();
    bool executeStatements(const QStringList &statements);

    QSqlDatabase m_database;
    QString m_connectionName;
    QString m_databasePath;
    QString m_errorString;
};

