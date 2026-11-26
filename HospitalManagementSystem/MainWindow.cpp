#include "MainWindow.h"

#include "DatabaseManager.h"
#include "PatientEditorDialog.h"
#include "PatientTableModel.h"
#include "SexDelegate.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QStackedWidget>
#include <QTableView>
#include <QToolBar>
#include <QToolButton>
#include <QUuid>
#include <QVBoxLayout>

namespace
{
QLabel *makeTitle(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("pageTitle"));
    return label;
}

QLabel *makeSubtitle(const QString &text)
{
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("pageSubtitle"));
    label->setWordWrap(true);
    return label;
}

QPushButton *makeButton(const QString &text, const QString &objectName, bool primary)
{
    auto *button = new QPushButton(text);
    button->setObjectName(objectName);
    button->setProperty("primary", primary);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}
} // namespace

MainWindow::MainWindow(const QString &databasePath, QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("仁心诊疗信息管理系统"));
    resize(1180, 760);
    setMinimumSize(1040, 680);

    m_stack = new QStackedWidget(this);
    m_stack->setObjectName(QStringLiteral("mainStack"));
    setCentralWidget(m_stack);
    m_stack->addWidget(buildLoginPage());

    buildToolbar();
    openDatabase(databaseFilePath(databasePath));
    if (DatabaseManager::instance().isOpen()) {
        buildAuthenticatedPages();
    } else {
        m_loginButton->setEnabled(false);
        setLoginMessage(DatabaseManager::instance().errorString(), true);
    }

    setStyleSheet(QStringLiteral(
        "QMainWindow, QWidget#rootPage { background: #F4F7FB; }"
        "QWidget { font-family: 'Microsoft YaHei UI'; font-size: 10pt; color: #24384C; }"
        "QToolBar#mainToolbar { background: #17324D; border: none; spacing: 4px;"
        "  padding: 8px 14px; }"
        "QToolBar#mainToolbar QToolButton { color: #DCE7F5; background: transparent;"
        "  border: none; border-radius: 7px; padding: 9px 14px; font-weight: 600; }"
        "QToolBar#mainToolbar QToolButton:hover { background: #254966; color: white; }"
        "QToolBar#mainToolbar QToolButton:checked { background: #2F6FED; color: white; }"
        "QLabel#toolbarBrand { color: white; font-size: 12pt; font-weight: 700;"
        "  padding-right: 16px; }"
        "QLabel#toolbarUser { color: #B8CAE0; padding: 0 10px; }"
        "QLabel#pageTitle { color: #17324D; font-size: 22pt; font-weight: 700; }"
        "QLabel#pageSubtitle { color: #6B7C93; font-size: 10pt; }"
        "QFrame#contentCard { background: white; border: 1px solid #E4EAF2;"
        "  border-radius: 13px; }"
        "QLineEdit { background: white; border: 1px solid #D7E0EC; border-radius: 8px;"
        "  padding: 9px 12px; min-height: 22px; selection-background-color: #2F6FED; }"
        "QLineEdit:focus { border: 1px solid #2F6FED; }"
        "QPushButton { border-radius: 8px; padding: 9px 16px; font-weight: 600; }"
        "QPushButton[primary=\"true\"] { color: white; background: #2F6FED; border: none; }"
        "QPushButton[primary=\"true\"]:hover { background: #245BC8; }"
        "QPushButton[primary=\"false\"] { color: #36506A; background: white;"
        "  border: 1px solid #D7E0EC; }"
        "QPushButton[primary=\"false\"]:hover { background: #EEF3FA; }"
        "QPushButton#dangerButton { color: #C83B4D; background: #FFF4F5;"
        "  border: 1px solid #F3CCD2; }"
        "QPushButton#dangerButton:hover { background: #FFE7EA; }"
        "QPushButton[card=\"true\"] { background: white; border: 1px solid #E0E8F2;"
        "  border-radius: 14px; padding: 22px; text-align: left; font-size: 13pt;"
        "  color: #17324D; }"
        "QPushButton[card=\"true\"]:hover { border: 1px solid #7EA4F5;"
        "  background: #F8FAFF; }"
        "QTableView { background: white; alternate-background-color: #F9FBFE;"
        "  border: 1px solid #E3EAF3; border-radius: 9px; selection-background-color: #DCE9FF;"
        "  selection-color: #17324D; gridline-color: #EDF1F6; }"
        "QHeaderView::section { background: #F3F7FC; color: #597089; border: none;"
        "  border-bottom: 1px solid #DFE7F0; padding: 10px 8px; font-weight: 700; }"
        "QTableView::item { padding: 7px 8px; border-bottom: 1px solid #F0F3F7; }"
        "QLabel#loginError, QLabel#patientStatus { padding: 7px 10px; border-radius: 6px; }"
        "QLabel#patientStatus[error=\"true\"] { color: #C83B4D; background: #FFF0F2; }"
        "QLabel#patientStatus[error=\"false\"] { color: #2E7D5B; background: #EDF8F2; }"
        "QStatusBar { background: #F4F7FB; color: #6B7C93; }"));
}

void MainWindow::buildToolbar()
{
    m_toolbar = addToolBar(QStringLiteral("主导航"));
    m_toolbar->setObjectName(QStringLiteral("mainToolbar"));
    m_toolbar->setMovable(false);
    m_toolbar->setFloatable(false);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_toolbar->hide();

    auto *brand = new QLabel(QStringLiteral("仁心诊疗"));
    brand->setObjectName(QStringLiteral("toolbarBrand"));
    m_toolbar->addWidget(brand);

    auto *dashboardAction = m_toolbar->addAction(QStringLiteral("工作台"));
    dashboardAction->setObjectName(QStringLiteral("dashboardAction"));
    connect(dashboardAction, &QAction::triggered, this, &MainWindow::showDashboard);

    m_patientAction = m_toolbar->addAction(QStringLiteral("患者管理"));
    m_patientAction->setObjectName(QStringLiteral("patientAction"));
    m_patientAction->setCheckable(true);
    connect(m_patientAction, &QAction::triggered, this, &MainWindow::navigateToPatients);

    m_doctorAction = m_toolbar->addAction(QStringLiteral("医生信息"));
    m_doctorAction->setObjectName(QStringLiteral("doctorAction"));
    m_doctorAction->setCheckable(true);
    connect(m_doctorAction, &QAction::triggered, this, &MainWindow::navigateToDoctors);

    m_departmentAction = m_toolbar->addAction(QStringLiteral("科室信息"));
    m_departmentAction->setObjectName(QStringLiteral("departmentAction"));
    m_departmentAction->setCheckable(true);
    connect(m_departmentAction, &QAction::triggered, this, &MainWindow::navigateToDepartments);

    m_historyAction = m_toolbar->addAction(QStringLiteral("操作记录"));
    m_historyAction->setObjectName(QStringLiteral("historyAction"));
    m_historyAction->setCheckable(true);
    connect(m_historyAction, &QAction::triggered, this, &MainWindow::navigateToHistory);

    auto *spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_toolbar->addWidget(spacer);

    m_toolbarUserLabel = new QLabel;
    m_toolbarUserLabel->setObjectName(QStringLiteral("toolbarUser"));
    m_toolbar->addWidget(m_toolbarUserLabel);

    auto *logoutButton = new QPushButton(QStringLiteral("退出登录"));
    logoutButton->setObjectName(QStringLiteral("logoutButton"));
    logoutButton->setCursor(Qt::PointingHandCursor);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::logout);
    m_toolbar->addWidget(logoutButton);
}

QWidget *MainWindow::buildLoginPage()
{
    auto *page = new QWidget;
    page->setObjectName(QStringLiteral("rootPage"));
    auto *outer = new QHBoxLayout(page);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto *visualPanel = new QWidget(page);
    visualPanel->setObjectName(QStringLiteral("loginVisual"));
    visualPanel->setMinimumWidth(530);
    visualPanel->setStyleSheet(QStringLiteral(
        "QWidget#loginVisual { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        " stop:0 #17324D, stop:1 #245BC8); }"
        "QLabel { color: white; }"));
    auto *visualLayout = new QVBoxLayout(visualPanel);
    visualLayout->setContentsMargins(58, 56, 58, 48);
    visualLayout->setSpacing(16);
    auto *eyebrow = new QLabel(QStringLiteral("HOSPITAL INFORMATION SYSTEM"));
    eyebrow->setStyleSheet(QStringLiteral(
        "font-size: 9pt; letter-spacing: 2px; color: #9DC0F7; font-weight: 700;"));
    visualLayout->addWidget(eyebrow);
    auto *heroTitle = new QLabel(QStringLiteral("让患者信息管理\n更清晰、更可靠"));
    heroTitle->setStyleSheet(QStringLiteral(
        "font-size: 28pt; font-weight: 700; line-height: 1.25;"));
    visualLayout->addWidget(heroTitle);
    auto *heroText = new QLabel(
        QStringLiteral("基于 Qt SQL 与 Model/View 架构\n连接登录、检索、维护与追溯的完整操作闭环。"));
    heroText->setStyleSheet(QStringLiteral(
        "font-size: 11pt; color: #D3E1F3; line-height: 1.6;"));
    heroText->setWordWrap(true);
    visualLayout->addWidget(heroText);
    visualLayout->addStretch();

    auto *featureLine = new QLabel(
        QStringLiteral("患者档案  ·  数据校验  ·  操作留痕"));
    featureLine->setStyleSheet(QStringLiteral(
        "font-size: 10pt; color: #BFD5F4; padding-top: 12px;"
        "border-top: 1px solid rgba(255,255,255,60);"));
    visualLayout->addWidget(featureLine);
    auto *developer = new QLabel(
        QStringLiteral("开发学生：林佳年  2024414290218\n软件工程 2 班"));
    developer->setObjectName(QStringLiteral("developerInfo"));
    developer->setStyleSheet(QStringLiteral(
        "font-size: 9.5pt; color: #C9DDF8; line-height: 1.5;"));
    visualLayout->addWidget(developer);
    outer->addWidget(visualPanel, 11);

    auto *formPanel = new QWidget(page);
    formPanel->setObjectName(QStringLiteral("rootPage"));
    auto *formOuter = new QVBoxLayout(formPanel);
    formOuter->setContentsMargins(72, 64, 72, 56);
    formOuter->addStretch();

    auto *card = new QFrame;
    card->setObjectName(QStringLiteral("loginCard"));
    card->setMaximumWidth(430);
    card->setStyleSheet(QStringLiteral(
        "QFrame#loginCard { background: white; border: 1px solid #E1E8F2;"
        "  border-radius: 18px; }"
        "QLabel#loginTitle { color: #17324D; font-size: 22pt; font-weight: 700; }"
        "QLabel#loginSubtitle { color: #708198; font-size: 10pt; }"
        "QLabel#fieldLabel { color: #3D536A; font-weight: 600; }"
        "QLabel#demoAccount { color: #6B7C93; background: #F3F7FC;"
        "  border-radius: 7px; padding: 9px; }"));
    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(32);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(20, 50, 80, 35));
    card->setGraphicsEffect(shadow);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(36, 32, 36, 30);
    cardLayout->setSpacing(12);

    auto *loginTitle = new QLabel(QStringLiteral("欢迎使用诊疗系统"));
    loginTitle->setObjectName(QStringLiteral("loginTitle"));
    cardLayout->addWidget(loginTitle);
    auto *loginSubtitle = new QLabel(QStringLiteral("请使用管理员账号登录后继续"));
    loginSubtitle->setObjectName(QStringLiteral("loginSubtitle"));
    cardLayout->addWidget(loginSubtitle);
    cardLayout->addSpacing(12);

    auto *usernameLabel = new QLabel(QStringLiteral("用户名"));
    usernameLabel->setObjectName(QStringLiteral("fieldLabel"));
    cardLayout->addWidget(usernameLabel);
    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setObjectName(QStringLiteral("loginUsernameEdit"));
    m_usernameEdit->setPlaceholderText(QStringLiteral("请输入用户名"));
    cardLayout->addWidget(m_usernameEdit);

    auto *passwordLabel = new QLabel(QStringLiteral("密码"));
    passwordLabel->setObjectName(QStringLiteral("fieldLabel"));
    cardLayout->addWidget(passwordLabel);
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setObjectName(QStringLiteral("loginPasswordEdit"));
    m_passwordEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    cardLayout->addWidget(m_passwordEdit);

    m_loginMessage = new QLabel;
    m_loginMessage->setObjectName(QStringLiteral("loginError"));
    m_loginMessage->setWordWrap(true);
    m_loginMessage->hide();
    cardLayout->addWidget(m_loginMessage);

    m_loginButton = new QPushButton(QStringLiteral("登 录"));
    m_loginButton->setObjectName(QStringLiteral("loginButton"));
    m_loginButton->setProperty("primary", true);
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_loginButton->setMinimumHeight(44);
    m_loginButton->setStyleSheet(QStringLiteral(
        "QPushButton { color: white; background: #2F6FED; border: none;"
        "  border-radius: 9px; font-size: 11pt; font-weight: 700; }"
        "QPushButton:hover { background: #245BC8; }"));
    connect(m_loginButton, &QPushButton::clicked, this, &MainWindow::attemptLogin);
    connect(m_usernameEdit, &QLineEdit::returnPressed, m_loginButton, &QPushButton::click);
    connect(m_passwordEdit, &QLineEdit::returnPressed, m_loginButton, &QPushButton::click);
    cardLayout->addWidget(m_loginButton);

    auto *demoAccount = new QLabel(
        QStringLiteral("演示账号  admin    密码  admin123"));
    demoAccount->setObjectName(QStringLiteral("demoAccount"));
    demoAccount->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(demoAccount);
    formOuter->addWidget(card, 0, Qt::AlignHCenter);
    formOuter->addStretch();
    outer->addWidget(formPanel, 9);

    return page;
}

QWidget *MainWindow::buildPageHeader(const QString &title, const QString &subtitle)
{
    auto *header = new QWidget;
    auto *layout = new QVBoxLayout(header);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    layout->addWidget(makeTitle(title));
    layout->addWidget(makeSubtitle(subtitle));
    return header;
}

QWidget *MainWindow::buildDashboardPage()
{
    auto *page = new QWidget;
    page->setObjectName(QStringLiteral("rootPage"));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(42, 36, 42, 34);
    layout->setSpacing(24);
    layout->addWidget(buildPageHeader(
        QStringLiteral("诊疗工作台"),
        QStringLiteral("登录成功。请选择需要进入的信息管理模块，所有数据库操作都会自动写入操作记录。")));

    auto *cards = new QHBoxLayout;
    cards->setSpacing(18);
    struct CardInfo {
        QString title;
        QString details;
        QString objectName;
    };
    const QList<CardInfo> infos = {
        {QStringLiteral("患者管理\n\n查询、新增、修改与删除患者档案"),
         QStringLiteral("核心功能"), QStringLiteral("navPatientsButton")},
        {QStringLiteral("医生信息\n\n查看医生与所属科室的关联数据"),
         QStringLiteral("外键关联"), QStringLiteral("navDoctorsButton")},
        {QStringLiteral("科室信息\n\n查看当前登记的全部诊疗科室"),
         QStringLiteral("基础数据"), QStringLiteral("navDepartmentsButton")},
        {QStringLiteral("操作记录\n\n追溯登录及患者数据维护过程"),
         QStringLiteral("操作留痕"), QStringLiteral("navHistoryButton")}
    };
    for (const CardInfo &info : infos) {
        auto *button = new QPushButton(info.title);
        button->setObjectName(info.objectName);
        button->setProperty("card", true);
        button->setMinimumHeight(178);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        cards->addWidget(button);
    }
    auto *patientCard = page->findChild<QPushButton *>(QStringLiteral("navPatientsButton"));
    auto *doctorCard = page->findChild<QPushButton *>(QStringLiteral("navDoctorsButton"));
    auto *departmentCard = page->findChild<QPushButton *>(QStringLiteral("navDepartmentsButton"));
    auto *historyCard = page->findChild<QPushButton *>(QStringLiteral("navHistoryButton"));
    connect(patientCard, &QPushButton::clicked, this, &MainWindow::navigateToPatients);
    connect(doctorCard, &QPushButton::clicked, this, &MainWindow::navigateToDoctors);
    connect(departmentCard, &QPushButton::clicked, this, &MainWindow::navigateToDepartments);
    connect(historyCard, &QPushButton::clicked, this, &MainWindow::navigateToHistory);
    layout->addLayout(cards);

    auto *summary = new QFrame;
    summary->setObjectName(QStringLiteral("contentCard"));
    auto *summaryLayout = new QHBoxLayout(summary);
    summaryLayout->setContentsMargins(24, 19, 24, 19);
    auto *left = new QVBoxLayout;
    left->setSpacing(3);
    auto *summaryTitle = new QLabel(QStringLiteral("实验三 · QtSQL 数据库模块与 View/Model"));
    summaryTitle->setStyleSheet(QStringLiteral("font-size: 12pt; font-weight: 700; color: #17324D;"));
    left->addWidget(summaryTitle);
    auto *summaryText = new QLabel(
        QStringLiteral("采用 QStackedWidget 管理页面，QSqlTableModel/QTableView 绑定患者数据，"
                       "QSqlQuery 完成登录与操作记录写入。"));
    summaryText->setObjectName(QStringLiteral("pageSubtitle"));
    summaryText->setWordWrap(true);
    left->addWidget(summaryText);
    summaryLayout->addLayout(left, 1);
    auto *student = new QLabel(
        QStringLiteral("开发者\n林佳年 · 2024414290218"));
    student->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    student->setStyleSheet(QStringLiteral("color: #2F6FED; font-weight: 600;"));
    summaryLayout->addWidget(student);
    layout->addWidget(summary);
    layout->addStretch();
    return page;
}

QWidget *MainWindow::buildPatientPage()
{
    auto *page = new QWidget;
    page->setObjectName(QStringLiteral("rootPage"));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(36, 30, 36, 28);
    layout->setSpacing(16);
    layout->addWidget(buildPageHeader(
        QStringLiteral("患者信息管理"),
        QStringLiteral("支持按姓名、身份证号或手机号查询，双击性别列可直接修改。")));

    auto *tools = new QHBoxLayout;
    tools->setSpacing(10);
    m_searchEdit = new QLineEdit;
    m_searchEdit->setObjectName(QStringLiteral("patientSearchEdit"));
    m_searchEdit->setPlaceholderText(QStringLiteral("输入姓名、身份证号或手机号"));
    m_searchEdit->setClearButtonEnabled(true);
    tools->addWidget(m_searchEdit, 1);

    auto *queryButton = makeButton(QStringLiteral("查询"), QStringLiteral("queryPatientButton"), true);
    auto *refreshButton = makeButton(
        QStringLiteral("刷新"), QStringLiteral("refreshPatientButton"), false);
    tools->addWidget(queryButton);
    tools->addWidget(refreshButton);
    layout->addLayout(tools);

    auto *tableCard = new QFrame;
    tableCard->setObjectName(QStringLiteral("contentCard"));
    auto *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(14, 14, 14, 12);
    tableLayout->setSpacing(10);

    m_patientView = new QTableView;
    m_patientView->setObjectName(QStringLiteral("patientTableView"));
    m_patientView->setModel(m_patientModel);
    m_patientView->setItemDelegateForColumn(PatientTableModel::SexColumn,
                                             new SexDelegate(m_patientView));
    m_patientView->setAlternatingRowColors(true);
    m_patientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_patientView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_patientView->setEditTriggers(QAbstractItemView::DoubleClicked
                                   | QAbstractItemView::SelectedClicked);
    m_patientView->setSortingEnabled(true);
    m_patientView->verticalHeader()->setVisible(false);
    m_patientView->verticalHeader()->setDefaultSectionSize(38);
    m_patientView->horizontalHeader()->setMinimumSectionSize(70);
    m_patientView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_patientView->horizontalHeader()->setSectionResizeMode(PatientTableModel::NameColumn,
                                                            QHeaderView::Stretch);
    m_patientView->horizontalHeader()->setSectionResizeMode(PatientTableModel::CreatedAtColumn,
                                                            QHeaderView::Stretch);
    m_patientView->setColumnHidden(PatientTableModel::IdColumn, true);
    tableLayout->addWidget(m_patientView, 1);

    auto *bottom = new QHBoxLayout;
    m_patientStatus = new QLabel;
    m_patientStatus->setObjectName(QStringLiteral("patientStatus"));
    bottom->addWidget(m_patientStatus, 1);
    auto *addButton = makeButton(QStringLiteral("新增患者"), QStringLiteral("addPatientButton"), true);
    auto *editButton = makeButton(QStringLiteral("编辑"), QStringLiteral("editPatientButton"), false);
    auto *deleteButton = makeButton(QStringLiteral("删除"), QStringLiteral("deletePatientButton"), false);
    deleteButton->setStyleSheet(QStringLiteral(
        "QPushButton { color: #C83B4D; background: #FFF4F5;"
        "  border: 1px solid #F3CCD2; border-radius: 8px; padding: 9px 16px;"
        "  font-weight: 600; }"
        "QPushButton:hover { background: #FFE7EA; }"));
    bottom->addWidget(addButton);
    bottom->addWidget(editButton);
    bottom->addWidget(deleteButton);
    tableLayout->addLayout(bottom);
    layout->addWidget(tableCard, 1);

    connect(queryButton, &QPushButton::clicked, this, &MainWindow::queryPatients);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MainWindow::queryPatients);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshPatientView);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addPatient);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editPatient);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deletePatient);
    connect(m_patientView, &QTableView::doubleClicked, this, &MainWindow::editPatient);

    return page;
}

QWidget *MainWindow::buildReferencePage(const QString &title,
                                        const QString &subtitle,
                                        QSqlQueryModel **model,
                                        const QString &query,
                                        const QStringList &headers)
{
    auto *page = new QWidget;
    page->setObjectName(QStringLiteral("rootPage"));
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(36, 30, 36, 28);
    layout->setSpacing(16);
    layout->addWidget(buildPageHeader(title, subtitle));

    auto *card = new QFrame;
    card->setObjectName(QStringLiteral("contentCard"));
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(14, 14, 14, 14);
    auto *view = new QTableView;
    view->setAlternatingRowColors(true);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->verticalHeader()->setVisible(false);
    view->verticalHeader()->setDefaultSectionSize(40);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->setModel(*model);
    cardLayout->addWidget(view);
    layout->addWidget(card, 1);

    (*model)->setQuery(query, DatabaseManager::instance().database());
    for (int i = 0; i < headers.size(); ++i) {
        (*model)->setHeaderData(i, Qt::Horizontal, headers.at(i));
    }
    if ((*model)->lastError().isValid()) {
        auto *error = new QLabel(QStringLiteral("查询失败：%1").arg((*model)->lastError().text()));
        error->setObjectName(QStringLiteral("patientStatus"));
        error->setProperty("error", true);
        layout->addWidget(error);
    }
    return page;
}

void MainWindow::buildAuthenticatedPages()
{
    m_patientModel = new PatientTableModel(this);
    m_patientModel->setObjectName(QStringLiteral("patientTableModel"));
    m_patientModel->refresh();

    m_stack->addWidget(buildDashboardPage());

    m_doctorModel = new QSqlQueryModel(this);
    m_departmentModel = new QSqlQueryModel(this);
    m_historyModel = new QSqlQueryModel(this);

    m_stack->addWidget(buildPatientPage());
    m_stack->addWidget(buildReferencePage(
        QStringLiteral("医生信息"),
        QStringLiteral("医生记录通过 DEPARTMENT_ID 外键与科室数据关联。"),
        &m_doctorModel,
        QStringLiteral(
            "SELECT d.EMPLOYEENO, d.NAME, dep.NAME AS DEPARTMENT "
            "FROM Doctor d JOIN Department dep ON dep.ID = d.DEPARTMENT_ID "
            "ORDER BY d.EMPLOYEENO"),
        {QStringLiteral("工号"), QStringLiteral("医生姓名"), QStringLiteral("所属科室")}));
    m_stack->addWidget(buildReferencePage(
        QStringLiteral("科室信息"),
        QStringLiteral("科室表是医生表的基础数据来源。"),
        &m_departmentModel,
        QStringLiteral("SELECT ID, NAME FROM Department ORDER BY NAME"),
        {QStringLiteral("科室编号"), QStringLiteral("科室名称")}));
    m_stack->addWidget(buildReferencePage(
        QStringLiteral("操作记录"),
        QStringLiteral("登录以及患者增、删、改操作都会写入 History 表。"),
        &m_historyModel,
        QStringLiteral(
            "SELECT h.TIMESTAMP, u.FULLNAME AS OPERATOR, h.EVENT "
            "FROM History h JOIN User u ON u.ID = h.USER_ID "
            "ORDER BY h.TIMESTAMP DESC LIMIT 200"),
        {QStringLiteral("时间"), QStringLiteral("操作用户"), QStringLiteral("事件")}));

}

void MainWindow::openDatabase(const QString &databasePath)
{
    DatabaseManager::instance().open(databasePath);
}

QString MainWindow::databaseFilePath(const QString &requestedPath) const
{
    if (!requestedPath.trimmed().isEmpty()) {
        return requestedPath;
    }
    return QDir(QCoreApplication::applicationDirPath()).filePath(
        QStringLiteral("hospital.sqlite"));
}

void MainWindow::attemptLogin()
{
    setLoginMessage(QString(), false);
    if (!DatabaseManager::instance().isOpen()) {
        setLoginMessage(DatabaseManager::instance().errorString(), true);
        return;
    }
    if (m_usernameEdit->text().trimmed().isEmpty()
        || m_passwordEdit->text().isEmpty()) {
        setLoginMessage(QStringLiteral("用户名和密码均不能为空。"), true);
        return;
    }

    QString error;
    if (!DatabaseManager::instance().authenticate(
            m_usernameEdit->text(), m_passwordEdit->text(),
            &m_currentUserId, &m_currentFullName, &error)) {
        setLoginMessage(error, true);
        m_passwordEdit->selectAll();
        m_passwordEdit->setFocus();
        return;
    }

    m_passwordEdit->clear();
    m_toolbar->show();
    m_toolbarUserLabel->setText(
        QStringLiteral("%1  |  林佳年 2024414290218").arg(m_currentFullName));
    m_patientModel->refresh();
    updatePatientStatus();
    logOperation(QStringLiteral("用户登录系统"));
    showDashboard();
}

void MainWindow::logout()
{
    if (!m_currentUserId.isEmpty()) {
        logOperation(QStringLiteral("用户退出系统"));
    }
    m_currentUserId.clear();
    m_currentFullName.clear();
    m_toolbar->hide();
    m_usernameEdit->clear();
    m_passwordEdit->clear();
    setLoginMessage(QString(), false);
    m_stack->setCurrentIndex(0);
    m_usernameEdit->setFocus();
}

void MainWindow::setLoginMessage(const QString &message, bool error)
{
    m_loginMessage->setText(message);
    m_loginMessage->setVisible(!message.isEmpty());
    m_loginMessage->setStyleSheet(error
        ? QStringLiteral("color: #C83B4D; background: #FFF0F2;"
                         "border: 1px solid #F6C9CF; border-radius: 7px; padding: 9px;")
        : QStringLiteral("color: #2E7D5B; background: #EDF8F2;"
                         "border: 1px solid #C9E8D7; border-radius: 7px; padding: 9px;"));
}

void MainWindow::showDashboard()
{
    m_stack->setCurrentIndex(1);
    m_patientAction->setChecked(false);
    m_doctorAction->setChecked(false);
    m_departmentAction->setChecked(false);
    m_historyAction->setChecked(false);
}

void MainWindow::navigateToPatients()
{
    m_stack->setCurrentIndex(2);
    m_patientAction->setChecked(true);
    m_doctorAction->setChecked(false);
    m_departmentAction->setChecked(false);
    m_historyAction->setChecked(false);
    m_patientModel->refresh();
    updatePatientStatus();
    m_searchEdit->setFocus();
}

void MainWindow::navigateToDoctors()
{
    m_stack->setCurrentIndex(3);
    m_patientAction->setChecked(false);
    m_doctorAction->setChecked(true);
    m_departmentAction->setChecked(false);
    m_historyAction->setChecked(false);
}

void MainWindow::navigateToDepartments()
{
    m_stack->setCurrentIndex(4);
    m_patientAction->setChecked(false);
    m_doctorAction->setChecked(false);
    m_departmentAction->setChecked(true);
    m_historyAction->setChecked(false);
}

void MainWindow::navigateToHistory()
{
    m_historyModel->setQuery(
        QStringLiteral(
            "SELECT h.TIMESTAMP, u.FULLNAME AS OPERATOR, h.EVENT "
            "FROM History h JOIN User u ON u.ID = h.USER_ID "
            "ORDER BY h.TIMESTAMP DESC LIMIT 200"),
        DatabaseManager::instance().database());
    m_stack->setCurrentIndex(5);
    m_patientAction->setChecked(false);
    m_doctorAction->setChecked(false);
    m_departmentAction->setChecked(false);
    m_historyAction->setChecked(true);
}

void MainWindow::queryPatients()
{
    QString error;
    const QString keyword = m_searchEdit->text().trimmed();
    if (!m_patientModel->refresh(keyword, &error)) {
        m_patientStatus->setText(error);
        m_patientStatus->setProperty("error", true);
        m_patientStatus->style()->unpolish(m_patientStatus);
        m_patientStatus->style()->polish(m_patientStatus);
        return;
    }
    if (m_patientModel->rowCount() == 0) {
        updatePatientStatus(
            keyword.isEmpty()
                ? QStringLiteral("暂无患者记录。")
                : QStringLiteral("未找到与“%1”匹配的患者，请更换查询条件。").arg(keyword));
        m_patientStatus->setProperty("error", true);
    } else {
        updatePatientStatus(QStringLiteral("查询完成，共找到 %1 条记录。")
                                .arg(m_patientModel->rowCount()));
    }
    m_patientStatus->style()->unpolish(m_patientStatus);
    m_patientStatus->style()->polish(m_patientStatus);
}

void MainWindow::refreshPatientView()
{
    m_searchEdit->clear();
    m_patientModel->refresh();
    updatePatientStatus();
}

void MainWindow::addPatient()
{
    PatientEditorDialog dialog(this);
    dialog.setAddMode();
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    PatientData data = dialog.patient();
    data.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    data.createdTimestamp = QDateTime::currentDateTime().toString(
        QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    QString error;
    if (!m_patientModel->addPatient(data, &error)) {
        QMessageBox::warning(this, QStringLiteral("新增失败"),
                             error + QStringLiteral("\n请检查身份证号是否重复。"));
        return;
    }
    m_searchEdit->clear();
    m_patientModel->refresh();
    updatePatientStatus(QStringLiteral("患者“%1”已新增，列表已刷新。").arg(data.name));
    logOperation(QStringLiteral("新增患者：%1（%2）").arg(data.name, data.idCard));
}

void MainWindow::editPatient()
{
    const QModelIndex index = m_patientView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, QStringLiteral("请选择记录"),
                                 QStringLiteral("请先在表格中选择要编辑的患者。"));
        return;
    }
    const PatientData original = patientFromCurrentRow();
    PatientEditorDialog dialog(this);
    dialog.setPatient(original);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    PatientData updated = dialog.patient();
    updated.id = original.id;
    updated.createdTimestamp = original.createdTimestamp;
    QString error;
    if (!m_patientModel->updatePatient(original.id, updated, &error)) {
        QMessageBox::warning(this, QStringLiteral("修改失败"), error);
        return;
    }
    updatePatientStatus(QStringLiteral("患者“%1”的信息已更新，列表已刷新。")
                            .arg(updated.name));
    logOperation(QStringLiteral("修改患者：%1（%2）").arg(updated.name, updated.idCard));
}

void MainWindow::deletePatient()
{
    const QModelIndex index = m_patientView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::information(this, QStringLiteral("请选择记录"),
                                 QStringLiteral("请先在表格中选择要删除的患者。"));
        return;
    }

    const PatientData data = patientFromCurrentRow();
    QMessageBox confirmation(this);
    confirmation.setWindowTitle(QStringLiteral("确认删除"));
    confirmation.setIcon(QMessageBox::Question);
    confirmation.setText(
        QStringLiteral("确定删除患者“%1”吗？\n该操作提交后不可撤销。").arg(data.name));
    auto *deleteChoice = confirmation.addButton(
        QStringLiteral("删除"), QMessageBox::DestructiveRole);
    auto *cancelChoice = confirmation.addButton(
        QStringLiteral("取消"), QMessageBox::RejectRole);
    confirmation.setDefaultButton(cancelChoice);
    confirmation.exec();
    if (confirmation.clickedButton() != deleteChoice) {
        updatePatientStatus(QStringLiteral("已取消删除，数据未发生变化。"));
        return;
    }

    QString error;
    if (!m_patientModel->removePatient(data.id, &error)) {
        QMessageBox::warning(this, QStringLiteral("删除失败"), error);
        return;
    }
    updatePatientStatus(QStringLiteral("患者“%1”已删除，列表已刷新。").arg(data.name));
    logOperation(QStringLiteral("删除患者：%1（%2）").arg(data.name, data.idCard));
}

void MainWindow::updatePatientStatus(const QString &message)
{
    if (!m_patientStatus) return;
    m_patientStatus->setText(
        message.isEmpty()
            ? QStringLiteral("当前显示 %1 条患者记录。").arg(m_patientModel->rowCount())
            : message);
    m_patientStatus->setProperty("error", false);
    m_patientStatus->style()->unpolish(m_patientStatus);
    m_patientStatus->style()->polish(m_patientStatus);
}

void MainWindow::logOperation(const QString &event)
{
    if (!m_currentUserId.isEmpty()) {
        QString error;
        DatabaseManager::instance().logEvent(m_currentUserId, event, &error);
    }
}

PatientData MainWindow::patientFromCurrentRow() const
{
    PatientData data;
    const QModelIndex index = m_patientView->currentIndex();
    if (!index.isValid()) return data;
    const QSqlRecord record = m_patientModel->record(index.row());
    data.id = record.value(QStringLiteral("ID")).toString();
    data.idCard = record.value(QStringLiteral("ID_CARD")).toString();
    data.name = record.value(QStringLiteral("NAME")).toString();
    data.sex = record.value(QStringLiteral("SEX")).toInt();
    data.dateOfBirth = record.value(QStringLiteral("DOB")).toString();
    data.height = record.value(QStringLiteral("HEIGHT")).toDouble();
    data.weight = record.value(QStringLiteral("WEIGHT")).toDouble();
    data.mobilePhone = record.value(QStringLiteral("MOBILEPHONE")).toString();
    data.age = record.value(QStringLiteral("AGE")).toInt();
    data.createdTimestamp = record.value(QStringLiteral("CREATEDTIMESTAMP")).toString();
    return data;
}

bool MainWindow::insertPatient(const PatientData &patient, QString *error)
{
    return m_patientModel->addPatient(patient, error);
}

PatientTableModel *MainWindow::patientModel() const
{
    return m_patientModel;
}

