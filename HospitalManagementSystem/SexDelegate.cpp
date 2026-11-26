#include "SexDelegate.h"

#include "PatientTableModel.h"

#include <QComboBox>

SexDelegate::SexDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *SexDelegate::createEditor(QWidget *parent,
                                   const QStyleOptionViewItem &,
                                   const QModelIndex &) const
{
    auto *editor = new QComboBox(parent);
    editor->addItem(QStringLiteral("男"), 1);
    editor->addItem(QStringLiteral("女"), 2);
    return editor;
}

void SexDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto *combo = qobject_cast<QComboBox *>(editor);
    if (!combo) return;
    const int value = index.data(Qt::EditRole).toInt();
    combo->setCurrentIndex(combo->findData(value));
}

void SexDelegate::setModelData(QWidget *editor,
                               QAbstractItemModel *model,
                               const QModelIndex &index) const
{
    auto *combo = qobject_cast<QComboBox *>(editor);
    if (!combo) return;
    model->setData(index, combo->currentData(), Qt::EditRole);

    auto *sqlModel = qobject_cast<PatientTableModel *>(model);
    if (sqlModel && !sqlModel->submitAll()) {
        sqlModel->revertAll();
    }
}

void SexDelegate::updateEditorGeometry(QWidget *editor,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

