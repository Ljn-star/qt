#pragma once

#include "PatientData.h"

#include <QMainWindow>

class QAction;
class QLabel;
class QLineEdit;
class QPushButton;
class QSqlQueryModel;
class QStackedWidget;
class QTableView;
class QToolBar;
class PatientTableModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &databasePath = QString(), QWidget *parent = nullptr);

    bool insertPatient(const PatientData &patient, QString *error = nullptr);
    PatientTableModel *patientModel() const;

private slots:
    void attemptLogin();
    void logout();
    void navigateToPatients();
    void navigateToDoctors();
    void navigateToDepartments();
    void navigateToHistory();
    void showDashboard();
    void queryPatients();
    void addPatient();
    void editPatient();
    void deletePatient();
    void refreshPatientView();

private:
    QWidget *buildLoginPage();
    QWidget *buildDashboardPage();
    QWidget *buildPatientPage();
    QWidget *buildReferencePage(const QString &title,
                                const QString &subtitle,
                                QSqlQueryModel **model,
                                const QString &query,
                                const QStringList &headers);
    QWidget *buildPageHeader(const QString &title, const QString &subtitle);
    void buildToolbar();
    void buildAuthenticatedPages();
    void openDatabase(const QString &databasePath);
    void setLoginMessage(const QString &message, bool error);
    void updatePatientStatus(const QString &message = QString());
    void logOperation(const QString &event);
    PatientData patientFromCurrentRow() const;
    QString databaseFilePath(const QString &requestedPath) const;

    QString m_databasePath;
    QString m_currentUserId;
    QString m_currentFullName;
    QStackedWidget *m_stack = nullptr;
    QToolBar *m_toolbar = nullptr;
    QAction *m_patientAction = nullptr;
    QAction *m_doctorAction = nullptr;
    QAction *m_departmentAction = nullptr;
    QAction *m_historyAction = nullptr;
    QLabel *m_toolbarUserLabel = nullptr;
    QPushButton *m_loginButton = nullptr;
    QLineEdit *m_usernameEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    QLabel *m_loginMessage = nullptr;
    QLabel *m_patientStatus = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QTableView *m_patientView = nullptr;
    PatientTableModel *m_patientModel = nullptr;
    QSqlQueryModel *m_doctorModel = nullptr;
    QSqlQueryModel *m_departmentModel = nullptr;
    QSqlQueryModel *m_historyModel = nullptr;
};

