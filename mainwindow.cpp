#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QStatusBar>
#include <QStringConverter>
#include <QTextDocument>
#include <QTextStream>
#include <QToolBar>

namespace {
constexpr qint64 kMaxOpenFileSize = 20 * 1024 * 1024; // 20MB，避免误开特大文件卡死
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupIcons();
    createStatusBarWidgets();
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // 初始界面状态与 Action 保持一致
    ui->textEdit->setLineNumberVisible(ui->actionShowLineNumber->isChecked());
    ui->textEdit->setLineWrapMode(ui->actionWordWrap->isChecked()
                                      ? QPlainTextEdit::WidgetWidth
                                      : QPlainTextEdit::NoWrap);

    // 修改状态 -> 窗口标题前的 "*"
    connect(ui->textEdit->document(), &QTextDocument::modificationChanged,
            this, [this](bool modified) { setWindowModified(modified); });

    // 编辑动作的使能状态与当前编辑状态保持一致
    connect(ui->textEdit, &QPlainTextEdit::undoAvailable,
            ui->actionUndo, &QAction::setEnabled);
    connect(ui->textEdit, &QPlainTextEdit::redoAvailable,
            ui->actionRedo, &QAction::setEnabled);
    connect(ui->textEdit, &QPlainTextEdit::copyAvailable,
            ui->actionCopy, &QAction::setEnabled);
    connect(ui->textEdit, &QPlainTextEdit::copyAvailable,
            ui->actionCut, &QAction::setEnabled);
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updateEditActions);

    // 状态栏信息
    connect(ui->textEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::updateStatusBar);
    connect(ui->textEdit, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusBar);

    updateWindowTitle();
    updateStatusBar();
    updateEditActions();
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    ui->actionCut->setEnabled(false);
    ui->actionCopy->setEnabled(false);
    setWindowModified(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupIcons()
{
    ui->actionNew->setIcon(QIcon(QStringLiteral(":/icons/new.svg")));
    ui->actionOpen->setIcon(QIcon(QStringLiteral(":/icons/open.svg")));
    ui->actionSave->setIcon(QIcon(QStringLiteral(":/icons/save.svg")));
    ui->actionSaveAs->setIcon(QIcon(QStringLiteral(":/icons/saveas.svg")));
    ui->actionUndo->setIcon(QIcon(QStringLiteral(":/icons/undo.svg")));
    ui->actionRedo->setIcon(QIcon(QStringLiteral(":/icons/redo.svg")));
    ui->actionCut->setIcon(QIcon(QStringLiteral(":/icons/cut.svg")));
    ui->actionCopy->setIcon(QIcon(QStringLiteral(":/icons/copy.svg")));
    ui->actionPaste->setIcon(QIcon(QStringLiteral(":/icons/paste.svg")));
    ui->actionFind->setIcon(QIcon(QStringLiteral(":/icons/find.svg")));
    ui->actionReplace->setIcon(QIcon(QStringLiteral(":/icons/replace.svg")));
    ui->actionFont->setIcon(QIcon(QStringLiteral(":/icons/font.svg")));
    ui->actionFontColor->setIcon(QIcon(QStringLiteral(":/icons/fontcolor.svg")));
    ui->actionTextBgColor->setIcon(QIcon(QStringLiteral(":/icons/bgcolor.svg")));
    ui->actionEditorBgColor->setIcon(QIcon(QStringLiteral(":/icons/bgcolor.svg")));
    ui->actionShowLineNumber->setIcon(QIcon(QStringLiteral(":/icons/linenumber.svg")));
    ui->actionWordWrap->setIcon(QIcon(QStringLiteral(":/icons/wrap.svg")));
}

void MainWindow::createStatusBarWidgets()
{
    m_statusLabel = new QLabel(this);
    m_cursorLabel = new QLabel(this);
    m_authorLabel = new QLabel(QStringLiteral("林佳年"), this);

    ui->statusBar->addPermanentWidget(m_statusLabel);
    ui->statusBar->addPermanentWidget(m_cursorLabel);
    ui->statusBar->addPermanentWidget(m_authorLabel);
}

void MainWindow::updateWindowTitle()
{
    const QString name = m_currentFile.isEmpty()
                             ? tr("新建文本文件")
                             : QFileInfo(m_currentFile).fileName();
    setWindowTitle(tr("%1[*] - 编辑器").arg(name));
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    m_currentFile = fileName;
    ui->textEdit->document()->setModified(false);
    updateWindowTitle();
}

void MainWindow::updateStatusBar()
{
    if (!m_statusLabel || !m_cursorLabel)
        return;

    const int length = ui->textEdit->toPlainText().length();
    const int lines = ui->textEdit->document()->blockCount();
    m_statusLabel->setText(tr("length: %1    lines: %2").arg(length).arg(lines));

    const QTextCursor cursor = ui->textEdit->textCursor();
    m_cursorLabel->setText(tr("Ln: %1    Col: %2")
                               .arg(cursor.blockNumber() + 1)
                               .arg(cursor.columnNumber() + 1));
}

void MainWindow::updateEditActions()
{
    const bool hasClipboardText = !QApplication::clipboard()->text().isEmpty();
    ui->actionPaste->setEnabled(hasClipboardText);
}

// ---------------------------------------------------------------------------
// 文件操作
// ---------------------------------------------------------------------------
void MainWindow::on_actionNew_triggered()
{
    if (!maybeSave())
        return;

    ui->textEdit->clear();
    ui->textEdit->document()->clearUndoRedoStacks();
    setCurrentFile(QString());
    updateStatusBar();
    ui->textEdit->setFocus();
}

void MainWindow::on_actionOpen_triggered()
{
    if (!maybeSave())
        return;

    const QString fileName = QFileDialog::getOpenFileName(
        this, tr("打开文件"), QString(),
        tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (fileName.isEmpty())
        return;

    loadFile(fileName);
}

void MainWindow::on_actionSave_triggered()
{
    if (m_currentFile.isEmpty()) {
        on_actionSaveAs_triggered();
        return;
    }
    saveFile(m_currentFile);
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("另存为"),
        m_currentFile.isEmpty() ? QStringLiteral("未命名.txt") : m_currentFile,
        tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (fileName.isEmpty())
        return;

    if (QFileInfo(fileName).suffix().isEmpty())
        fileName += QStringLiteral(".txt");
    saveFile(fileName);
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

bool MainWindow::loadFile(const QString &fileName)
{
    const QFileInfo info(fileName);
    if (!info.exists() || !info.isFile()) {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("文件不存在或不是普通文件：\n%1").arg(fileName));
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("无法读取文件：\n%1\n%2")
                                 .arg(fileName, file.errorString()));
        return false;
    }

    if (file.size() > kMaxOpenFileSize) {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("文件超过 20MB，为免卡顿不再打开：\n%1").arg(fileName));
        return false;
    }

    const QByteArray raw = file.readAll();
    file.close();

    if (raw.contains('\0')) {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("该文件包含二进制数据，不是纯文本文件：\n%1").arg(fileName));
        return false;
    }

    // Windows 文本文件多为 CRLF，读入后统一规范为 \n，避免编辑器里出现多余字符
    QString text = QString::fromUtf8(raw);
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    ui->textEdit->setPlainText(text);
    setCurrentFile(fileName);
    updateStatusBar();
    ui->textEdit->setFocus();
    return true;
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("保存失败"),
                             tr("无法写入文件：\n%1\n%2")
                                 .arg(fileName, file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << ui->textEdit->toPlainText();
    out.flush();

    const bool ok = (out.status() == QTextStream::Ok) && (file.error() == QFile::NoError);
    file.close();

    if (!ok) {
        QMessageBox::warning(this, tr("保存失败"),
                             tr("写入文件时发生错误：\n%1").arg(fileName));
        return false;
    }

    setCurrentFile(fileName);
    return true;
}

bool MainWindow::maybeSave()
{
    if (!ui->textEdit->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("提示"), tr("文档已修改，是否保存？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        if (m_currentFile.isEmpty()) {
            on_actionSaveAs_triggered();
            return !ui->textEdit->document()->isModified();
        }
        return saveFile(m_currentFile);
    }
    return ret != QMessageBox::Cancel;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

// ---------------------------------------------------------------------------
// 编辑操作
// ---------------------------------------------------------------------------
void MainWindow::on_actionUndo_triggered() { ui->textEdit->undo(); }
void MainWindow::on_actionRedo_triggered() { ui->textEdit->redo(); }
void MainWindow::on_actionCut_triggered() { ui->textEdit->cut(); }
void MainWindow::on_actionCopy_triggered() { ui->textEdit->copy(); }
void MainWindow::on_actionPaste_triggered() { ui->textEdit->paste(); }
void MainWindow::on_actionSelectAll_triggered() { ui->textEdit->selectAll(); }

// ---------------------------------------------------------------------------
// 格式设置
// ---------------------------------------------------------------------------
void MainWindow::on_actionFont_triggered()
{
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, ui->textEdit->font(), this, tr("选择字体"));
    if (ok)
        ui->textEdit->setFont(font);
}

void MainWindow::on_actionFontColor_triggered()
{
    QColor initial = ui->textEdit->currentCharFormat().foreground().color();
    if (!initial.isValid())
        initial = Qt::black;

    const QColor color = QColorDialog::getColor(initial, this, tr("选择字体颜色"));
    if (!color.isValid())
        return;

    QTextCharFormat format;
    format.setForeground(color);
    ui->textEdit->mergeCurrentCharFormat(format);
}

void MainWindow::on_actionTextBgColor_triggered()
{
    QColor initial = ui->textEdit->currentCharFormat().background().color();
    if (!initial.isValid())
        initial = Qt::yellow;

    const QColor color = QColorDialog::getColor(initial, this, tr("选择字体背景色"));
    if (!color.isValid())
        return;

    QTextCharFormat format;
    format.setBackground(color);
    ui->textEdit->mergeCurrentCharFormat(format);
}

void MainWindow::on_actionEditorBgColor_triggered()
{
    const QColor color = QColorDialog::getColor(
        ui->textEdit->palette().color(QPalette::Base), this, tr("选择编辑器背景色"));
    if (color.isValid())
        ui->textEdit->setEditorBackgroundColor(color);
}

void MainWindow::on_actionWordWrap_toggled(bool checked)
{
    ui->textEdit->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth
                                          : QPlainTextEdit::NoWrap);
}

void MainWindow::on_actionShowLineNumber_toggled(bool checked)
{
    ui->textEdit->setLineNumberVisible(checked);
}

// ---------------------------------------------------------------------------
// 查看
// ---------------------------------------------------------------------------
void MainWindow::on_actionShowToolBar_toggled(bool checked)
{
    ui->mainToolBar->setVisible(checked);
}

void MainWindow::on_actionShowStatusBar_toggled(bool checked)
{
    ui->statusBar->setVisible(checked);
}

// ---------------------------------------------------------------------------
// 帮助
// ---------------------------------------------------------------------------
void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dialog(this);
    dialog.exec();
}

// ---------------------------------------------------------------------------
// 查找 / 替换
// ---------------------------------------------------------------------------
void MainWindow::on_actionFind_triggered()
{
    if (!m_findDialog) {
        m_findDialog = new FindDialog(this, ui->textEdit);
        m_findDialog->setAttribute(Qt::WA_DeleteOnClose);
    }
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::on_actionReplace_triggered()
{
    if (!m_replaceDialog) {
        m_replaceDialog = new ReplaceDialog(this, ui->textEdit);
        m_replaceDialog->setAttribute(Qt::WA_DeleteOnClose);
    }
    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
}
