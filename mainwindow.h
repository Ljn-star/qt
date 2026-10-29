#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FindDialog;
class ReplaceDialog;
class QCloseEvent;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // 文件读写（同时便于单元测试直接调用）
    bool loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // ---- 文件 ----
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();

    // ---- 编辑 ----
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionSelectAll_triggered();

    // ---- 格式 ----
    void on_actionFont_triggered();
    void on_actionFontColor_triggered();
    void on_actionTextBgColor_triggered();
    void on_actionEditorBgColor_triggered();
    void on_actionWordWrap_toggled(bool checked);
    void on_actionShowLineNumber_toggled(bool checked);

    // ---- 查看 ----
    void on_actionShowToolBar_toggled(bool checked);
    void on_actionShowStatusBar_toggled(bool checked);

    // ---- 帮助 ----
    void on_actionAbout_triggered();

    // ---- 查找 / 替换 ----
    void on_actionFind_triggered();
    void on_actionReplace_triggered();

    void updateStatusBar();
    void updateEditActions();

private:
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    void updateWindowTitle();
    void setupIcons();
    void createStatusBarWidgets();

    Ui::MainWindow *ui;
    QString m_currentFile;
    QLabel *m_statusLabel = nullptr;   // length / lines
    QLabel *m_cursorLabel = nullptr;   // Ln / Col
    QLabel *m_authorLabel = nullptr;
    QPointer<FindDialog> m_findDialog;
    QPointer<ReplaceDialog> m_replaceDialog;
};

#endif // MAINWINDOW_H
