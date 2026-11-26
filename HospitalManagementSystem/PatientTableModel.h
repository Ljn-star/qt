#pragma once

#include "PatientData.h"

#include <QSqlTableModel>

class PatientTableModel : public QSqlTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        IdColumn = 0,
        IdCardColumn,
        NameColumn,
        SexColumn,
        BirthDateColumn,
        HeightColumn,
        WeightColumn,
        MobileColumn,
        AgeColumn,
        CreatedAtColumn,
        ColumnCount
    };

    explicit PatientTableModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    bool refresh(const QString &keyword = QString(), QString *error = nullptr);
    bool addPatient(const PatientData &patient, QString *error = nullptr);
    bool updatePatient(const QString &id, const PatientData &patient, QString *error = nullptr);
    bool removePatient(const QString &id, QString *error = nullptr);
    int rowForId(const QString &id) const;
    QString idAt(int row) const;

private:
    int fieldIndex(const QString &fieldName) const;
    void setError(QString *error, const QString &message) const;
};

