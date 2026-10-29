#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>
#include <QTextDocument>

QT_BEGIN_NAMESPACE
namespace Ui { class ReplaceDialog; }
QT_END_NAMESPACE

class QPlainTextEdit;

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent, QPlainTextEdit *editor);
    ~ReplaceDialog() override;

    void setSearchText(const QString &text);

private slots:
    void on_btnFindNext_clicked();
    void on_btnReplace_clicked();
    void on_btnReplaceAll_clicked();
    void on_btnCancel_clicked();

private:
    bool findNext();
    bool selectionMatches(const QString &target) const;
    QTextDocument::FindFlags findFlags() const;

    Ui::ReplaceDialog *ui;
    QPlainTextEdit *m_editor;
};

#endif // REPLACEDIALOG_H
