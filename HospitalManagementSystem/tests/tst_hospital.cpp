#include "../DatabaseManager.h"
#include "../PatientData.h"
#include "../PatientTableModel.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTemporaryDir>
#include <QTest>

class HospitalTests : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void loginAcceptsAndRejectsCredentials();
    void requiredFieldsAndDuplicateDataAreRejected();
    void queryWithoutResultKeepsModelConsistent();
    void crudKeepsDatabaseAndViewInSync();
    void invalidDatabasePathReportsError();

private:
    PatientData samplePatient(const QString &idCard,
                              const QString &name,
                              const QString &mobile) const;

    std::unique_ptr<QTemporaryDir> m_tempDirectory;
};

void HospitalTests::init()
{
    m_tempDirectory = std::make_unique<QTemporaryDir>();
    QVERIFY(m_tempDirectory->isValid());
    DatabaseManager::instance().close();
    QVERIFY2(DatabaseManager::instance().open(
                 m_tempDirectory->filePath(QStringLiteral("hospital.sqlite"))),
             qPrintable(DatabaseManager::instance().errorString()));
}

void HospitalTests::cleanup()
{
    DatabaseManager::instance().close();
    m_tempDirectory.reset();
}

void HospitalTests::loginAcceptsAndRejectsCredentials()
{
    QString userId;
    QString fullName;
    QString error;

    QVERIFY(!DatabaseManager::instance().authenticate(
        QStringLiteral("admin"), QStringLiteral("bad-password"),
        &userId, &fullName, &error));
    QVERIFY(error.contains(QStringLiteral("错误")));

    QVERIFY2(DatabaseManager::instance().authenticate(
                 QStringLiteral("admin"), QStringLiteral("admin123"),
                 &userId, &fullName, &error),
             qPrintable(error));
    QVERIFY(!userId.isEmpty());
    QCOMPARE(fullName, QStringLiteral("系统管理员"));
}

void HospitalTests::requiredFieldsAndDuplicateDataAreRejected()
{
    QString validationError;
    PatientData missing = samplePatient(
        QStringLiteral("441900200001011234"), QString(), QStringLiteral("13800138009"));
    QVERIFY(!PatientRules::validate(missing, &validationError));
    QVERIFY(validationError.contains(QStringLiteral("姓名")));

    PatientTableModel model;
    PatientData first = samplePatient(
        QStringLiteral("441900200001011234"),
        QStringLiteral("测试患者"), QStringLiteral("13800138009"));
    QString databaseError;
    QVERIFY2(model.addPatient(first, &databaseError), qPrintable(databaseError));

    PatientData duplicate = first;
    duplicate.id = QStringLiteral("P_DUPLICATE_TEST");
    QVERIFY(!model.addPatient(duplicate, &databaseError));
    QVERIFY(!databaseError.isEmpty());

    QSqlQuery query(DatabaseManager::instance().database());
    QVERIFY(query.exec(QStringLiteral(
        "SELECT COUNT(*) FROM Patient WHERE ID_CARD = '441900200001011234'")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 1);
}

void HospitalTests::queryWithoutResultKeepsModelConsistent()
{
    PatientTableModel model;
    const int originalCount = model.rowCount();
    QVERIFY(model.rowCount() >= 2);

    QString error;
    QVERIFY2(model.refresh(QStringLiteral("不会有这个患者"), &error), qPrintable(error));
    QCOMPARE(model.rowCount(), 0);

    QVERIFY2(model.refresh(QString(), &error), qPrintable(error));
    QCOMPARE(model.rowCount(), originalCount);
}

void HospitalTests::crudKeepsDatabaseAndViewInSync()
{
    PatientTableModel model;
    const int initialCount = model.rowCount();
    PatientData patient = samplePatient(
        QStringLiteral("441900200202021234"),
        QStringLiteral("增删改测试"), QStringLiteral("13800138008"));

    QString error;
    QVERIFY2(model.addPatient(patient, &error), qPrintable(error));
    QCOMPARE(model.rowCount(), initialCount + 1);
    QVERIFY(model.rowForId(patient.id) >= 0);

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(QStringLiteral("SELECT NAME FROM Patient WHERE ID = :id"));
    query.bindValue(QStringLiteral(":id"), patient.id);
    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toString(), patient.name);

    patient.name = QStringLiteral("增删改测试-已修改");
    patient.weight = 68.3;
    QVERIFY2(model.updatePatient(patient.id, patient, &error), qPrintable(error));
    QCOMPARE(model.record(model.rowForId(patient.id))
                 .value(QStringLiteral("NAME")).toString(),
             patient.name);

    query.prepare(QStringLiteral("SELECT NAME, WEIGHT FROM Patient WHERE ID = :id"));
    query.bindValue(QStringLiteral(":id"), patient.id);
    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toString(), patient.name);
    QCOMPARE(query.value(1).toDouble(), 68.3);

    QVERIFY2(model.removePatient(patient.id, &error), qPrintable(error));
    QCOMPARE(model.rowCount(), initialCount);
    QVERIFY(model.rowForId(patient.id) < 0);

    query.prepare(QStringLiteral("SELECT COUNT(*) FROM Patient WHERE ID = :id"));
    query.bindValue(QStringLiteral(":id"), patient.id);
    QVERIFY(query.exec());
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);
}

void HospitalTests::invalidDatabasePathReportsError()
{
    DatabaseManager::instance().close();
    QVERIFY(!DatabaseManager::instance().open(m_tempDirectory->path()));
    QVERIFY(!DatabaseManager::instance().errorString().isEmpty());
}

PatientData HospitalTests::samplePatient(const QString &idCard,
                                         const QString &name,
                                         const QString &mobile) const
{
    PatientData patient;
    patient.id = QStringLiteral("P_TEST_%1").arg(idCard.right(4));
    patient.idCard = idCard;
    patient.name = name;
    patient.sex = 1;
    patient.dateOfBirth = QStringLiteral("2000-01-01");
    patient.height = 172.5;
    patient.weight = 65.0;
    patient.mobilePhone = mobile;
    patient.age = PatientRules::calculateAge(QDate(2000, 1, 1));
    patient.createdTimestamp = QStringLiteral("2026-11-26 12:00:00");
    return patient;
}

QTEST_GUILESS_MAIN(HospitalTests)

#include "tst_hospital.moc"
