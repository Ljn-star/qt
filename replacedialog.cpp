#include "replacedialog.h"
#include "ui_replacedialog.h"

#include <QMessageBox>
#include <QPlainTextEdit>
#include <QTextCursor>

ReplaceDialog::ReplaceDialog(QWidget *parent, QPlainTextEdit *editor)
    : QDialog(parent)
    , ui(new Ui::ReplaceDialog)
    , m_editor(editor)
{
    ui->setupUi(this);
    ui->lineEditTarget->setFocus();
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

void ReplaceDialog::setSearchText(const QString &text)
{
    ui->lineEditTarget->setText(text);
}

QTextDocument::FindFlags ReplaceDialog::findFlags() const
{
    QTextDocument::FindFlags flags;
    if (ui->checkCase->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (ui->radioUp->isChecked())
        flags |= QTextDocument::FindBackward;
    return flags;
}

bool ReplaceDialog::findNext()
{
    const QString target = ui->lineEditTarget->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入要查找的内容。"));
        return false;
    }

    const QTextDocument::FindFlags flags = findFlags();
    if (m_editor->find(target, flags))
        return true;

    // 回绕再找一次
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(ui->radioUp->isChecked() ? QTextCursor::End : QTextCursor::Start);
    m_editor->setTextCursor(cursor);
    if (m_editor->find(target, flags))
        return true;

    QMessageBox::information(this, tr("替换"), tr("未找到“%1”。").arg(target));
    return false;
}

bool ReplaceDialog::selectionMatches(const QString &target) const
{
    const QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection())
        return false;

    const QString selected = cursor.selectedText();
    const Qt::CaseSensitivity cs = ui->checkCase->isChecked() ? Qt::CaseSensitive
                                                              : Qt::CaseInsensitive;
    return selected.compare(target, cs) == 0;
}

void ReplaceDialog::on_btnFindNext_clicked()
{
    findNext();
    m_editor->setFocus();
}

void ReplaceDialog::on_btnReplace_clicked()
{
    const QString target = ui->lineEditTarget->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入要查找的内容。"));
        return;
    }

    if (selectionMatches(target)) {
        QTextCursor cursor = m_editor->textCursor();
        cursor.insertText(ui->lineEditReplace->text());
        m_editor->setTextCursor(cursor);
    }
    findNext();
}

void ReplaceDialog::on_btnReplaceAll_clicked()
{
    const QString target = ui->lineEditTarget->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入要查找的内容。"));
        return;
    }

    const QString replacement = ui->lineEditReplace->text();
    const QTextDocument::FindFlags flags = findFlags();

    QTextCursor cursor(m_editor->document());
    if (flags & QTextDocument::FindBackward)
        cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock(); // 让"全部替换"成为一次可撤销的操作

    int count = 0;
    QTextCursor found = m_editor->document()->find(target, cursor, flags);
    while (!found.isNull()) {
        found.insertText(replacement);
        ++count;
        // insertText 后游标位于替换文本之后，从这里继续查找可避免死循环
        found = m_editor->document()->find(target, found, flags);
    }
    cursor.endEditBlock();

    if (count > 0)
        QMessageBox::information(this, tr("替换"), tr("共替换 %1 处。").arg(count));
    else
        QMessageBox::information(this, tr("替换"), tr("未找到“%1”。").arg(target));
}

void ReplaceDialog::on_btnCancel_clicked()
{
    close();
}
