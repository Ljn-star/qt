#include "DatabaseManager.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager manager;
    return manager;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

bool DatabaseManager::open(const QString &databasePath)
{
    close();
    m_errorString.clear();
    m_databasePath = QFileInfo(databasePath).absoluteFilePath();

    const QFileInfo databaseInfo(m_databasePath);
    if (!QDir().mkpath(databaseInfo.absolutePath())) {
        m_errorString = QStringLiteral("无法创建数据库目录：%1").arg(databaseInfo.absolutePath());
        return false;
    }

    m_connectionName = QStringLiteral("hospital_sql_%1")
                           .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_database.setDatabaseName(m_databasePath);

    if (!m_database.open()) {
        m_errorString = QStringLiteral("数据库连接失败：%1").arg(m_database.lastError().text());
        return false;
    }

    QSqlQuery query(m_database);
    query.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
    query.exec(QStringLiteral("PRAGMA busy_timeout = 3000"));

    if (!createSchema() || !seedData()) {
        m_database.close();
        return false;
    }

    return true;
}

void DatabaseManager::close()
{
    if (!m_database.isValid()) {
        return;
    }

    if (m_database.isOpen()) {
        m_database.close();
    }

    const QString connectionName = m_connectionName;
    m_database = QSqlDatabase();
    m_connectionName.clear();
    if (!connectionName.isEmpty()) {
        QSqlDatabase::removeDatabase(connectionName);
    }
}

bool DatabaseManager::isOpen() const
{
    return m_database.isValid() && m_database.isOpen();
}

QSqlDatabase DatabaseManager::database() const
{
    return m_database;
}

QString DatabaseManager::databasePath() const
{
    return m_databasePath;
}

QString DatabaseManager::errorString() const
{
    return m_errorString;
}

bool DatabaseManager::createSchema()
{
    const QStringList statements = {
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS User ("
            "ID TEXT PRIMARY KEY NOT NULL,"
            "FULLNAME TEXT NOT NULL,"
            "USERNAME TEXT NOT NULL UNIQUE,"
            "PASSWORD TEXT NOT NULL)"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS Department ("
            "ID TEXT PRIMARY KEY NOT NULL,"
            "NAME TEXT NOT NULL UNIQUE)"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS Doctor ("
            "ID TEXT PRIMARY KEY NOT NULL,"
            "EMPLOYEENO TEXT NOT NULL UNIQUE,"
            "NAME TEXT NOT NULL,"
            "DEPARTMENT_ID TEXT NOT NULL,"
            "FOREIGN KEY (DEPARTMENT_ID) REFERENCES Department(ID))"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS Patient ("
            "ID TEXT PRIMARY KEY NOT NULL,"
            "ID_CARD TEXT NOT NULL UNIQUE,"
            "NAME TEXT NOT NULL,"
            "SEX INTEGER NOT NULL CHECK (SEX IN (1, 2)),"
            "DOB TEXT NOT NULL,"
            "HEIGHT REAL NOT NULL CHECK (HEIGHT BETWEEN 30 AND 250),"
            "WEIGHT REAL NOT NULL CHECK (WEIGHT BETWEEN 1 AND 300),"
            "MOBILEPHONE TEXT NOT NULL,"
            "AGE INTEGER NOT NULL CHECK (AGE BETWEEN 0 AND 150),"
            "CREATEDTIMESTAMP TEXT NOT NULL)"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS History ("
            "ID TEXT PRIMARY KEY NOT NULL,"
            "USER_ID TEXT NOT NULL,"
            "EVENT TEXT NOT NULL,"
            "TIMESTAMP TEXT NOT NULL,"
            "FOREIGN KEY (USER_ID) REFERENCES User(ID))")
    };

    return executeStatements(statements);
}

bool DatabaseManager::seedData()
{
    if (!m_database.transaction()) {
        m_errorString = QStringLiteral("无法启动初始化事务：%1")
                            .arg(m_database.lastError().text());
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT OR IGNORE INTO User(ID, FULLNAME, USERNAME, PASSWORD) "
        "VALUES(:id, :fullname, :username, :password)"));
    query.bindValue(QStringLiteral(":id"),
                    QStringLiteral("U0000000000000000000000000000001"));
    query.bindValue(QStringLiteral(":fullname"), QStringLiteral("系统管理员"));
    query.bindValue(QStringLiteral(":username"), QStringLiteral("admin"));
    query.bindValue(QStringLiteral(":password"), hashPassword(QStringLiteral("admin123")));
    if (!query.exec()) {
        m_errorString = QStringLiteral("初始化管理员失败：%1").arg(query.lastError().text());
        m_database.rollback();
        return false;
    }

    query.prepare(QStringLiteral(
        "INSERT OR IGNORE INTO Department(ID, NAME) VALUES(:id, :name)"));
    const QStringList departments = {
        QStringLiteral("D0000000000000000000000000000001|内科"),
        QStringLiteral("D0000000000000000000000000000002|外科"),
        QStringLiteral("D0000000000000000000000000000003|儿科")
    };
    for (const QString &entry : departments) {
        const QStringList parts = entry.split(u'|');
        query.bindValue(QStringLiteral(":id"), parts.at(0));
        query.bindValue(QStringLiteral(":name"), parts.at(1));
        if (!query.exec()) {
            m_errorString = QStringLiteral("初始化科室失败：%1").arg(query.lastError().text());
            m_database.rollback();
            return false;
        }
    }

    query.prepare(QStringLiteral(
        "INSERT OR IGNORE INTO Doctor(ID, EMPLOYEENO, NAME, DEPARTMENT_ID) "
        "VALUES(:id, :employeeNo, :name, :departmentId)"));
    query.bindValue(QStringLiteral(":id"),
                    QStringLiteral("C0000000000000000000000000000001"));
    query.bindValue(QStringLiteral(":employeeNo"), QStringLiteral("DOC-001"));
    query.bindValue(QStringLiteral(":name"), QStringLiteral("陈医生"));
    query.bindValue(QStringLiteral(":departmentId"),
                    QStringLiteral("D0000000000000000000000000000001"));
    if (!query.exec()) {
        m_errorString = QStringLiteral("初始化医生失败：%1").arg(query.lastError().text());
        m_database.rollback();
        return false;
    }

    query.prepare(QStringLiteral(
        "INSERT OR IGNORE INTO Patient("
        "ID, ID_CARD, NAME, SEX, DOB, HEIGHT, WEIGHT, MOBILEPHONE, AGE, CREATEDTIMESTAMP) "
        "VALUES(:id, :idCard, :name, :sex, :dob, :height, :weight, :mobile, :age, :created)"));
    const QList<QVariantList> patients = {
        {QStringLiteral("P0000000000000000000000000000001"),
         QStringLiteral("441900199203151234"), QStringLiteral("李敏"), 2,
         QStringLiteral("1992-03-15"), 165.0, 54.0, QStringLiteral("13800138001"), 34,
         QStringLiteral("2026-11-20 09:00:00")},
        {QStringLiteral("P0000000000000000000000000000002"),
         QStringLiteral("441900198807220018"), QStringLiteral("王强"), 1,
         QStringLiteral("1988-07-22"), 176.0, 72.0, QStringLiteral("13800138002"), 38,
         QStringLiteral("2026-11-21 10:30:00")}
    };
    for (const QVariantList &patient : patients) {
        query.bindValue(QStringLiteral(":id"), patient.at(0));
        query.bindValue(QStringLiteral(":idCard"), patient.at(1));
        query.bindValue(QStringLiteral(":name"), patient.at(2));
        query.bindValue(QStringLiteral(":sex"), patient.at(3));
        query.bindValue(QStringLiteral(":dob"), patient.at(4));
        query.bindValue(QStringLiteral(":height"), patient.at(5));
        query.bindValue(QStringLiteral(":weight"), patient.at(6));
        query.bindValue(QStringLiteral(":mobile"), patient.at(7));
        query.bindValue(QStringLiteral(":age"), patient.at(8));
        query.bindValue(QStringLiteral(":created"), patient.at(9));
        if (!query.exec()) {
            m_errorString = QStringLiteral("初始化患者失败：%1").arg(query.lastError().text());
            m_database.rollback();
            return false;
        }
    }

    if (!m_database.commit()) {
        m_errorString = QStringLiteral("提交初始化数据失败：%1")
                            .arg(m_database.lastError().text());
        return false;
    }

    return true;
}

bool DatabaseManager::executeStatements(const QStringList &statements)
{
    QSqlQuery query(m_database);
    for (const QString &statement : statements) {
        if (!query.exec(statement)) {
            m_errorString = QStringLiteral("创建数据库表失败：%1\nSQL：%2")
                                .arg(query.lastError().text(), statement);
            return false;
        }
    }
    return true;
}

bool DatabaseManager::authenticate(const QString &username,
                                   const QString &password,
                                   QString *userId,
                                   QString *fullName,
                                   QString *error)
{
    if (!isOpen()) {
        if (error) *error = QStringLiteral("数据库尚未连接。");
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT ID, FULLNAME FROM User "
        "WHERE USERNAME = :username AND PASSWORD = :password"));
    query.bindValue(QStringLiteral(":username"), username.trimmed());
    query.bindValue(QStringLiteral(":password"), hashPassword(password));

    if (!query.exec()) {
        if (error) *error = QStringLiteral("登录查询失败：%1").arg(query.lastError().text());
        return false;
    }
    if (!query.next()) {
        if (error) *error = QStringLiteral("用户名或密码错误，请重新输入。");
        return false;
    }

    if (userId) *userId = query.value(0).toString();
    if (fullName) *fullName = query.value(1).toString();
    if (error) error->clear();
    return true;
}

bool DatabaseManager::logEvent(const QString &userId,
                               const QString &event,
                               QString *error)
{
    if (!isOpen()) {
        if (error) *error = QStringLiteral("数据库尚未连接。");
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO History(ID, USER_ID, EVENT, TIMESTAMP) "
        "VALUES(:id, :userId, :event, :timestamp)"));
    query.bindValue(QStringLiteral(":id"),
                    QUuid::createUuid().toString(QUuid::WithoutBraces));
    query.bindValue(QStringLiteral(":userId"), userId);
    query.bindValue(QStringLiteral(":event"), event);
    query.bindValue(QStringLiteral(":timestamp"),
                    QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    if (!query.exec()) {
        if (error) *error = QStringLiteral("写入操作记录失败：%1")
                                .arg(query.lastError().text());
        return false;
    }
    return true;
}

QString DatabaseManager::hashPassword(const QString &password)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}
