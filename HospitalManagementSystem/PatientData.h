#pragma once

#include <QDate>
#include <QRegularExpression>
#include <QString>

struct PatientData
{
    QString id;
    QString idCard;
    QString name;
    int sex = 1;
    QString dateOfBirth;
    double height = 170.0;
    double weight = 65.0;
    QString mobilePhone;
    int age = 0;
    QString createdTimestamp;
};

namespace PatientRules
{
inline int calculateAge(const QDate &birthDate, const QDate &today = QDate::currentDate())
{
    int age = today.year() - birthDate.year();
    if (today.month() < birthDate.month()
        || (today.month() == birthDate.month() && today.day() < birthDate.day())) {
        --age;
    }
    return age;
}

inline bool validate(const PatientData &data, QString *message)
{
    const QString name = data.name.trimmed();
    if (name.isEmpty()) {
        if (message) *message = QStringLiteral("姓名不能为空。");
        return false;
    }
    if (name.size() > 30) {
        if (message) *message = QStringLiteral("姓名不能超过 30 个字符。");
        return false;
    }

    static const QRegularExpression idCardPattern(
        QStringLiteral("^[0-9]{17}[0-9Xx]$"));
    if (!idCardPattern.match(data.idCard.trimmed()).hasMatch()) {
        if (message) *message = QStringLiteral("身份证号应为 18 位，最后一位可为 X。");
        return false;
    }

    const QDate birthDate = QDate::fromString(data.dateOfBirth, QStringLiteral("yyyy-MM-dd"));
    if (!birthDate.isValid()) {
        if (message) *message = QStringLiteral("出生日期格式无效。");
        return false;
    }
    if (birthDate > QDate::currentDate()) {
        if (message) *message = QStringLiteral("出生日期不能晚于今天。");
        return false;
    }
    const int age = calculateAge(birthDate);
    if (age < 0 || age > 150) {
        if (message) *message = QStringLiteral("患者年龄应在 0 至 150 岁之间。");
        return false;
    }

    if (data.sex != 1 && data.sex != 2) {
        if (message) *message = QStringLiteral("请选择患者性别。");
        return false;
    }
    if (data.height < 30.0 || data.height > 250.0) {
        if (message) *message = QStringLiteral("身高应在 30 至 250 cm 之间。");
        return false;
    }
    if (data.weight < 1.0 || data.weight > 300.0) {
        if (message) *message = QStringLiteral("体重应在 1 至 300 kg 之间。");
        return false;
    }

    static const QRegularExpression mobilePattern(QStringLiteral("^[0-9]{11}$"));
    if (!mobilePattern.match(data.mobilePhone.trimmed()).hasMatch()) {
        if (message) *message = QStringLiteral("手机号应为 11 位数字。");
        return false;
    }

    return true;
}
} // namespace PatientRules

