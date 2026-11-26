#include "MainWindow.h"

#include "DatabaseManager.h"
#include "PatientData.h"
#include "PatientEditorDialog.h"
#include "PatientTableModel.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTime>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFont>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QTableView>
#include <QThread>
#include <QUuid>

namespace
{
void waitForUi(int milliseconds)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < milliseconds) {
        QApplication::processEvents(QEventLoop::AllEvents, 25);
        QThread::msleep(8);
    }
}

bool saveWidget(QWidget *widget, const QString &directory, const QString &name)
{
    QApplication::processEvents();
    return widget->grab().save(QDir(directory).filePath(name), "PNG");
}
} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QFont font(QStringLiteral("Microsoft YaHei UI"));
    font.setPointSize(10);
    app.setFont(font);

    const QString outputDirectory = QDir::current().filePath(QStringLiteral("screenshots"));
    QDir().mkpath(outputDirectory);
    const QString databasePath = QDir::current().filePath(QStringLiteral("capture.sqlite"));
    if (QFile::exists(databasePath)) {
        QFile::remove(databasePath);
    }

    MainWindow window(databasePath);
    window.resize(1180, 760);
    window.show();
    waitForUi(450);
    saveWidget(&window, outputDirectory, QStringLiteral("01_login.png"));

    auto *username = window.findChild<QLineEdit *>(QStringLiteral("loginUsernameEdit"));
    auto *password = window.findChild<QLineEdit *>(QStringLiteral("loginPasswordEdit"));
    auto *login = window.findChild<QPushButton *>(QStringLiteral("loginButton"));
    if (!username || !password || !login) return 2;

    username->setText(QStringLiteral("admin"));
    password->setText(QStringLiteral("wrong-password"));
    login->click();
    waitForUi(180);
    saveWidget(&window, outputDirectory, QStringLiteral("02_login_error.png"));

    password->setText(QStringLiteral("admin123"));
    login->click();
    waitForUi(250);
    saveWidget(&window, outputDirectory, QStringLiteral("03_dashboard.png"));

    auto *patientAction = window.findChild<QAction *>(QStringLiteral("patientAction"));
    if (!patientAction) return 3;
    patientAction->trigger();
    waitForUi(220);
    saveWidget(&window, outputDirectory, QStringLiteral("04_patient_list.png"));

    PatientEditorDialog editor;
    editor.resize(500, 560);
    editor.show();
    waitForUi(180);
    auto *idCard = editor.findChild<QLineEdit *>(QStringLiteral("idCardEdit"));
    auto *name = editor.findChild<QLineEdit *>(QStringLiteral("nameEdit"));
    auto *sex = editor.findChild<QComboBox *>(QStringLiteral("sexCombo"));
    auto *birthDate = editor.findChild<QDateEdit *>(QStringLiteral("birthDateEdit"));
    auto *height = editor.findChild<QDoubleSpinBox *>(QStringLiteral("heightSpin"));
    auto *weight = editor.findChild<QDoubleSpinBox *>(QStringLiteral("weightSpin"));
    auto *mobile = editor.findChild<QLineEdit *>(QStringLiteral("mobileEdit"));
    if (!idCard || !name || !sex || !birthDate || !height || !weight || !mobile) return 4;

    idCard->setText(QStringLiteral("441900199909091234"));
    name->setText(QStringLiteral("赵晓雨"));
    sex->setCurrentIndex(sex->findData(2));
    birthDate->setDate(QDate(1999, 9, 9));
    height->setValue(164.5);
    weight->setValue(52.8);
    mobile->setText(QStringLiteral("13800138003"));
    waitForUi(120);
    saveWidget(&editor, outputDirectory, QStringLiteral("05_patient_editor.png"));

    PatientData patient = editor.patient();
    patient.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    patient.createdTimestamp = QDateTime::currentDateTime().toString(
        QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    QString error;
    if (!window.insertPatient(patient, &error)) return 5;
    QSqlQuery adminQuery(DatabaseManager::instance().database());
    if (adminQuery.exec(QStringLiteral(
            "SELECT ID FROM User WHERE USERNAME = 'admin'")) && adminQuery.next()) {
        DatabaseManager::instance().logEvent(
            adminQuery.value(0).toString(),
            QStringLiteral("新增患者：%1（%2）").arg(patient.name, patient.idCard));
    }

    mobile->setText(QStringLiteral("123"));
    editor.accept();
    waitForUi(120);
    saveWidget(&editor, outputDirectory, QStringLiteral("06_validation_error.png"));
    editor.close();

    patientAction->trigger();
    waitForUi(180);
    auto *table = window.findChild<QTableView *>(QStringLiteral("patientTableView"));
    auto *model = window.findChild<PatientTableModel *>(QStringLiteral("patientTableModel"));
    if (!table || !model) return 6;
    const int addedRow = model->rowForId(patient.id);
    if (addedRow >= 0) {
        table->setCurrentIndex(model->index(addedRow, PatientTableModel::NameColumn));
        table->scrollTo(model->index(addedRow, PatientTableModel::NameColumn));
    }
    waitForUi(160);
    saveWidget(&window, outputDirectory, QStringLiteral("07_after_add.png"));

    QMessageBox confirmDialog(&window);
    confirmDialog.setWindowTitle(QStringLiteral("确认删除"));
    confirmDialog.setIcon(QMessageBox::Question);
    confirmDialog.setText(
        QStringLiteral("确定删除患者“赵晓雨”吗？\n该操作提交后不可撤销。"));
    confirmDialog.addButton(QStringLiteral("删除"), QMessageBox::DestructiveRole);
    auto *cancelChoice = confirmDialog.addButton(
        QStringLiteral("取消"), QMessageBox::RejectRole);
    confirmDialog.setDefaultButton(cancelChoice);
    confirmDialog.show();
    waitForUi(160);
    saveWidget(&confirmDialog, outputDirectory, QStringLiteral("08_delete_confirm.png"));
    confirmDialog.close();

    auto *search = window.findChild<QLineEdit *>(QStringLiteral("patientSearchEdit"));
    auto *queryButton = window.findChild<QPushButton *>(QStringLiteral("queryPatientButton"));
    if (!search || !queryButton) return 7;
    search->setText(QStringLiteral("不存在的患者"));
    queryButton->click();
    waitForUi(160);
    saveWidget(&window, outputDirectory, QStringLiteral("09_query_empty.png"));

    search->clear();
    queryButton->click();
    waitForUi(120);
    auto *historyAction = window.findChild<QAction *>(QStringLiteral("historyAction"));
    if (!historyAction) return 8;
    historyAction->trigger();
    waitForUi(180);
    saveWidget(&window, outputDirectory, QStringLiteral("10_history.png"));

    window.close();
    return 0;
}

