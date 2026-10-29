#include "finddialog.h"
#include "ui_finddialog.h"

#include <QMessageBox>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextDocument>

FindDialog::FindDialog(QWidget *parent, QPlainTextEdit *editor)
    : QDialog(parent)
    , ui(new Ui::FindDialog)
    , m_editor(editor)
{
    ui->setupUi(this);
    ui->lineEditTarget->setFocus();
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::setSearchText(const QString &text)
{
    ui->lineEditTarget->setText(text);
}

void FindDialog::on_btnFind_clicked()
{
    const QString target = ui->lineEditTarget->text();
    if (target.isEmpty()) {
        QMessageBox::information(this, tr("查找"), tr("请输入要查找的内容。"));
        return;
    }

    QTextDocument::FindFlags flags;
    if (ui->checkCase->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    const bool backward = ui->radioUp->isChecked();
    if (backward)
        flags |= QTextDocument::FindBackward;

    if (!m_editor->find(target, flags)) {
        // 找不到时从文档首/尾回绕再找一次，实现"连续查找"
        QTextCursor cursor = m_editor->textCursor();
        cursor.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
        m_editor->setTextCursor(cursor);
        if (!m_editor->find(target, flags)) {
            QMessageBox::information(this, tr("查找"), tr("未找到“%1”。").arg(target));
            return;
        }
    }
    m_editor->setFocus();
}

void FindDialog::on_btnCancel_clicked()
{
    close();
}
