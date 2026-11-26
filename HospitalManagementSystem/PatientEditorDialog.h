#pragma once

#include "PatientData.h"

#include <QDialog>

class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;

class PatientEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PatientEditorDialog(QWidget *parent = nullptr);

    void setAddMode();
    void setPatient(const PatientData &patient);
    PatientData patient() const;
    void accept() override;

private:
    void updateAgePreview();
    void showValidationError(const QString &message);
    PatientData collectData() const;

    bool m_editMode = false;
    QString m_id;
    QString m_createdTimestamp;
    QLabel *m_headerLabel = nullptr;
    QLineEdit *m_idCardEdit = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_sexCombo = nullptr;
    QDateEdit *m_birthDateEdit = nullptr;
    QDoubleSpinBox *m_heightSpin = nullptr;
    QDoubleSpinBox *m_weightSpin = nullptr;
    QLineEdit *m_mobileEdit = nullptr;
    QLabel *m_ageLabel = nullptr;
    QLabel *m_errorLabel = nullptr;
};

