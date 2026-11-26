#include "PatientTableModel.h"

#include "DatabaseManager.h"

#include <QBrush>
#include <QColor>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlTableModel>

PatientTableModel::PatientTableModel(QObject *parent)
    : QSqlTableModel(parent, DatabaseManager::instance().database())
{
    setTable(QStringLiteral("Patient"));
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    setSort(CreatedAtColumn, Qt::DescendingOrder);
    select();
}

QVariant PatientTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole && index.column() == SexColumn) {
        const int sex = QSqlTableModel::data(index, Qt::EditRole).toInt();
        return sex == 2 ? QStringLiteral("女") : QStringLiteral("男");
    }
    if (role == Qt::TextAlignmentRole) {
        if (index.column() == IdCardColumn || index.column() == MobileColumn) {
            return QVariant(Qt::AlignCenter);
        }
        if (index.column() != NameColumn && index.column() != CreatedAtColumn) {
            return QVariant(Qt::AlignCenter);
        }
    }
    if (role == Qt::ForegroundRole && index.column() == SexColumn) {
        const int sex = QSqlTableModel::data(index, Qt::EditRole).toInt();
        return QBrush(sex == 2 ? QColor(QStringLiteral("#C0447A"))
                               : QColor(QStringLiteral("#2F6FED")));
    }
    if (role == Qt::ToolTipRole && index.column() == CreatedAtColumn) {
        return QSqlTableModel::data(index, Qt::DisplayRole);
    }
    if (role == Qt::DisplayRole && index.column() == HeightColumn) {
        return QStringLiteral("%1").arg(QSqlTableModel::data(index, Qt::DisplayRole).toDouble(),
                                        0, 'f', 1);
    }
    if (role == Qt::DisplayRole && index.column() == WeightColumn) {
        return QStringLiteral("%1").arg(QSqlTableModel::data(index, Qt::DisplayRole).toDouble(),
                                        0, 'f', 1);
    }

    return QSqlTableModel::data(index, role);
}

QVariant PatientTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case IdColumn: return QStringLiteral("ID");
        case IdCardColumn: return QStringLiteral("身份证号");
        case NameColumn: return QStringLiteral("姓名");
        case SexColumn: return QStringLiteral("性别");
        case BirthDateColumn: return QStringLiteral("出生日期");
        case HeightColumn: return QStringLiteral("身高(cm)");
        case WeightColumn: return QStringLiteral("体重(kg)");
        case MobileColumn: return QStringLiteral("手机号");
        case AgeColumn: return QStringLiteral("年龄");
        case CreatedAtColumn: return QStringLiteral("创建时间");
        default: break;
        }
    }
    return QSqlTableModel::headerData(section, orientation, role);
}

bool PatientTableModel::refresh(const QString &keyword, QString *error)
{
    QString escaped = keyword.trimmed();
    escaped.replace(u'\'', QStringLiteral("''"));
    if (escaped.isEmpty()) {
        setFilter(QString());
    } else {
        setFilter(QStringLiteral(
            "NAME LIKE '%%1%' OR ID_CARD LIKE '%%1%' OR MOBILEPHONE LIKE '%%1%'")
                      .arg(escaped));
    }

    if (!select()) {
        setError(error, QStringLiteral("刷新患者列表失败：%1").arg(lastError().text()));
        return false;
    }
    if (error) error->clear();
    return true;
}

bool PatientTableModel::addPatient(const PatientData &patient, QString *error)
{
    QSqlRecord record = this->record();
    auto assign = [&record](const QString &field, const QVariant &value) {
        const int index = record.indexOf(field);
        if (index >= 0) record.setValue(index, value);
    };
    assign(QStringLiteral("ID"), patient.id);
    assign(QStringLiteral("ID_CARD"), patient.idCard.trimmed().toUpper());
    assign(QStringLiteral("NAME"), patient.name.trimmed());
    assign(QStringLiteral("SEX"), patient.sex);
    assign(QStringLiteral("DOB"), patient.dateOfBirth);
    assign(QStringLiteral("HEIGHT"), patient.height);
    assign(QStringLiteral("WEIGHT"), patient.weight);
    assign(QStringLiteral("MOBILEPHONE"), patient.mobilePhone.trimmed());
    assign(QStringLiteral("AGE"), patient.age);
    assign(QStringLiteral("CREATEDTIMESTAMP"), patient.createdTimestamp);

    if (!insertRecord(-1, record)) {
        setError(error, QStringLiteral("新增患者失败：%1").arg(lastError().text()));
        revertAll();
        return false;
    }
    if (!submitAll()) {
        setError(error, QStringLiteral("保存患者失败：%1").arg(lastError().text()));
        revertAll();
        return false;
    }
    if (!select()) {
        setError(error, QStringLiteral("刷新患者列表失败：%1").arg(lastError().text()));
        return false;
    }
    if (error) error->clear();
    return true;
}

bool PatientTableModel::updatePatient(const QString &id,
                                      const PatientData &patient,
                                      QString *error)
{
    const int row = rowForId(id);
    if (row < 0) {
        setError(error, QStringLiteral("未找到要修改的患者记录。"));
        return false;
    }

    auto write = [this, row](const QString &field, const QVariant &value) {
        const int column = fieldIndex(field);
        if (column >= 0) {
            setData(index(row, column), value);
        }
    };
    write(QStringLiteral("ID_CARD"), patient.idCard.trimmed().toUpper());
    write(QStringLiteral("NAME"), patient.name.trimmed());
    write(QStringLiteral("SEX"), patient.sex);
    write(QStringLiteral("DOB"), patient.dateOfBirth);
    write(QStringLiteral("HEIGHT"), patient.height);
    write(QStringLiteral("WEIGHT"), patient.weight);
    write(QStringLiteral("MOBILEPHONE"), patient.mobilePhone.trimmed());
    write(QStringLiteral("AGE"), patient.age);

    if (!submitAll()) {
        setError(error, QStringLiteral("更新患者失败：%1").arg(lastError().text()));
        revertAll();
        return false;
    }
    if (!select()) {
        setError(error, QStringLiteral("刷新患者列表失败：%1").arg(lastError().text()));
        return false;
    }
    if (error) error->clear();
    return true;
}

bool PatientTableModel::removePatient(const QString &id, QString *error)
{
    const int row = rowForId(id);
    if (row < 0) {
        setError(error, QStringLiteral("未找到要删除的患者记录。"));
        return false;
    }
    if (!removeRow(row)) {
        setError(error, QStringLiteral("删除患者失败：%1").arg(lastError().text()));
        revertAll();
        return false;
    }
    if (!submitAll()) {
        setError(error, QStringLiteral("提交删除操作失败：%1").arg(lastError().text()));
        revertAll();
        return false;
    }
    if (!select()) {
        setError(error, QStringLiteral("刷新患者列表失败：%1").arg(lastError().text()));
        return false;
    }
    if (error) error->clear();
    return true;
}

int PatientTableModel::rowForId(const QString &id) const
{
    for (int row = 0; row < rowCount(); ++row) {
        if (record(row).value(QStringLiteral("ID")).toString() == id) {
            return row;
        }
    }
    return -1;
}

QString PatientTableModel::idAt(int row) const
{
    if (row < 0 || row >= rowCount()) {
        return QString();
    }
    return record(row).value(QStringLiteral("ID")).toString();
}

int PatientTableModel::fieldIndex(const QString &fieldName) const
{
    return record().indexOf(fieldName);
}

void PatientTableModel::setError(QString *error, const QString &message) const
{
    if (error) *error = message;
}

