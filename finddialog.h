#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class FindDialog; }
QT_END_NAMESPACE

class QPlainTextEdit;

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent, QPlainTextEdit *editor);
    ~FindDialog() override;

    void setSearchText(const QString &text);

private slots:
    void on_btnFind_clicked();
    void on_btnCancel_clicked();

private:
    Ui::FindDialog *ui;
    QPlainTextEdit *m_editor;
};

#endif // FINDDIALOG_H
