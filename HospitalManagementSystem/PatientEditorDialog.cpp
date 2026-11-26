#include "PatientEditorDialog.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

PatientEditorDialog::PatientEditorDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("patientEditorDialog"));
    setWindowTitle(QStringLiteral("编辑患者信息"));
    setModal(true);
    setMinimumWidth(470);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(14);

    m_headerLabel = new QLabel(QStringLiteral("新增患者"), this);
    m_headerLabel->setObjectName(QStringLiteral("dialogTitle"));
    layout->addWidget(m_headerLabel);

    auto *subtitle = new QLabel(QStringLiteral("请填写完整信息，身份证号与手机号将参与数据校验。"), this);
    subtitle->setObjectName(QStringLiteral("dialogSubtitle"));
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(12);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_idCardEdit = new QLineEdit(this);
    m_idCardEdit->setObjectName(QStringLiteral("idCardEdit"));
    m_idCardEdit->setPlaceholderText(QStringLiteral("18 位身份证号"));
    m_idCardEdit->setMaxLength(18);
    form->addRow(QStringLiteral("身份证号"), m_idCardEdit);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setObjectName(QStringLiteral("nameEdit"));
    m_nameEdit->setPlaceholderText(QStringLiteral("患者姓名"));
    m_nameEdit->setMaxLength(30);
    form->addRow(QStringLiteral("姓名"), m_nameEdit);

    m_sexCombo = new QComboBox(this);
    m_sexCombo->setObjectName(QStringLiteral("sexCombo"));
    m_sexCombo->addItem(QStringLiteral("男"), 1);
    m_sexCombo->addItem(QStringLiteral("女"), 2);
    form->addRow(QStringLiteral("性别"), m_sexCombo);

    m_birthDateEdit = new QDateEdit(this);
    m_birthDateEdit->setObjectName(QStringLiteral("birthDateEdit"));
    m_birthDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    m_birthDateEdit->setCalendarPopup(true);
    m_birthDateEdit->setDate(QDate(1990, 1, 1));
    m_birthDateEdit->setMaximumDate(QDate::currentDate());
    form->addRow(QStringLiteral("出生日期"), m_birthDateEdit);

    m_ageLabel = new QLabel(this);
    m_ageLabel->setObjectName(QStringLiteral("agePreview"));
    form->addRow(QStringLiteral("年龄"), m_ageLabel);

    m_heightSpin = new QDoubleSpinBox(this);
    m_heightSpin->setObjectName(QStringLiteral("heightSpin"));
    m_heightSpin->setRange(30.0, 250.0);
    m_heightSpin->setDecimals(1);
    m_heightSpin->setSuffix(QStringLiteral(" cm"));
    m_heightSpin->setValue(170.0);
    form->addRow(QStringLiteral("身高"), m_heightSpin);

    m_weightSpin = new QDoubleSpinBox(this);
    m_weightSpin->setObjectName(QStringLiteral("weightSpin"));
    m_weightSpin->setRange(1.0, 300.0);
    m_weightSpin->setDecimals(1);
    m_weightSpin->setSuffix(QStringLiteral(" kg"));
    m_weightSpin->setValue(65.0);
    form->addRow(QStringLiteral("体重"), m_weightSpin);

    m_mobileEdit = new QLineEdit(this);
    m_mobileEdit->setObjectName(QStringLiteral("mobileEdit"));
    m_mobileEdit->setPlaceholderText(QStringLiteral("11 位手机号"));
    m_mobileEdit->setMaxLength(11);
    form->addRow(QStringLiteral("手机号"), m_mobileEdit);

    layout->addLayout(form);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setObjectName(QStringLiteral("formError"));
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    layout->addWidget(m_errorLabel);

    auto *buttons = new QHBoxLayout;
    buttons->addStretch();
    auto *cancelButton = new QPushButton(QStringLiteral("取消"), this);
    cancelButton->setObjectName(QStringLiteral("secondaryButton"));
    auto *saveButton = new QPushButton(QStringLiteral("保存"), this);
    saveButton->setObjectName(QStringLiteral("primaryButton"));
    saveButton->setDefault(true);
    buttons->addWidget(cancelButton);
    buttons->addWidget(saveButton);
    layout->addLayout(buttons);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, this, &PatientEditorDialog::accept);
    connect(m_birthDateEdit, &QDateEdit::dateChanged,
            this, &PatientEditorDialog::updateAgePreview);

    setStyleSheet(QStringLiteral(
        "QDialog { background: #F7F9FC; font-family: 'Microsoft YaHei UI'; font-size: 10pt; }"
        "QLabel#dialogTitle { font-size: 18pt; font-weight: 700; color: #17324D; }"
        "QLabel#dialogSubtitle { color: #6B7C93; padding-bottom: 6px; }"
        "QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox {"
        "  background: white; border: 1px solid #D7E0EC; border-radius: 7px;"
        "  padding: 8px 10px; min-height: 21px; color: #17324D; }"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDoubleSpinBox:focus {"
        "  border: 1px solid #2F6FED; }"
        "QLabel#agePreview { color: #2F6FED; font-weight: 600; padding: 8px 10px; }"
        "QLabel#formError { color: #C83B4D; background: #FFF0F2;"
        "  border: 1px solid #F6C9CF; border-radius: 6px; padding: 9px; }"
        "QPushButton { min-width: 72px; min-height: 34px; border-radius: 7px; font-weight: 600; }"
        "QPushButton#primaryButton { color: white; background: #2F6FED; border: none; }"
        "QPushButton#primaryButton:hover { background: #245BC8; }"
        "QPushButton#secondaryButton { color: #425466; background: white;"
        "  border: 1px solid #D7E0EC; }"
        "QPushButton#secondaryButton:hover { background: #EEF3FA; }"));

    setAddMode();
}

void PatientEditorDialog::setAddMode()
{
    m_editMode = false;
    m_id.clear();
    m_createdTimestamp.clear();
    setWindowTitle(QStringLiteral("新增患者信息"));
    m_headerLabel->setText(QStringLiteral("新增患者"));
    m_idCardEdit->clear();
    m_nameEdit->clear();
    m_sexCombo->setCurrentIndex(0);
    m_birthDateEdit->setDate(QDate(1990, 1, 1));
    m_heightSpin->setValue(170.0);
    m_weightSpin->setValue(65.0);
    m_mobileEdit->clear();
    m_errorLabel->hide();
    updateAgePreview();
    m_idCardEdit->setReadOnly(false);
    m_idCardEdit->setFocus();
}

void PatientEditorDialog::setPatient(const PatientData &patient)
{
    m_editMode = true;
    m_id = patient.id;
    m_createdTimestamp = patient.createdTimestamp;
    setWindowTitle(QStringLiteral("编辑患者信息"));
    m_headerLabel->setText(QStringLiteral("编辑患者"));
    m_idCardEdit->setText(patient.idCard);
    m_nameEdit->setText(patient.name);
    m_sexCombo->setCurrentIndex(m_sexCombo->findData(patient.sex));
    const QDate birthDate = QDate::fromString(patient.dateOfBirth, QStringLiteral("yyyy-MM-dd"));
    if (birthDate.isValid()) m_birthDateEdit->setDate(birthDate);
    m_heightSpin->setValue(patient.height);
    m_weightSpin->setValue(patient.weight);
    m_mobileEdit->setText(patient.mobilePhone);
    m_errorLabel->hide();
    updateAgePreview();
    m_nameEdit->setFocus();
    m_nameEdit->selectAll();
}

PatientData PatientEditorDialog::patient() const
{
    return collectData();
}

void PatientEditorDialog::accept()
{
    const PatientData data = collectData();
    QString message;
    if (!PatientRules::validate(data, &message)) {
        showValidationError(message);
        return;
    }
    QDialog::accept();
}

void PatientEditorDialog::updateAgePreview()
{
    const int age = PatientRules::calculateAge(m_birthDateEdit->date());
    m_ageLabel->setText(QStringLiteral("%1 岁（根据出生日期自动计算）").arg(age));
}

void PatientEditorDialog::showValidationError(const QString &message)
{
    m_errorLabel->setText(message);
    m_errorLabel->show();
}

PatientData PatientEditorDialog::collectData() const
{
    PatientData data;
    data.id = m_id;
    data.idCard = m_idCardEdit->text().trimmed().toUpper();
    data.name = m_nameEdit->text().trimmed();
    data.sex = m_sexCombo->currentData().toInt();
    data.dateOfBirth = m_birthDateEdit->date().toString(QStringLiteral("yyyy-MM-dd"));
    data.height = m_heightSpin->value();
    data.weight = m_weightSpin->value();
    data.mobilePhone = m_mobileEdit->text().trimmed();
    data.age = PatientRules::calculateAge(m_birthDateEdit->date());
    data.createdTimestamp = m_createdTimestamp;
    return data;
}
