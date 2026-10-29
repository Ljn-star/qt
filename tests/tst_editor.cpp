#include <QtTest>

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStatusBar>
#include <QTemporaryDir>
#include <QTimer>

#include "mainwindow.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

namespace {

QLabel *statusLabelContaining(MainWindow &window, const QString &needle)
{
    auto *bar = window.findChild<QStatusBar *>(QStringLiteral("statusBar"));
    if (!bar)
        return nullptr;
    const auto labels = bar->findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (label->text().contains(needle))
            return label;
    }
    return nullptr;
}

// 非模态文件对话框出现后自动选择路径并确认
void acceptFileDialogLater(const QString &path, int attempts = 30)
{
    QTimer::singleShot(80, [path, attempts]() {
        if (auto *dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget())) {
            dialog->selectFile(path);
            // 等文件名真正填入编辑框后再点"确定/保存"，避免时序竞争
            QTimer::singleShot(150, dialog, [dialog]() {
                if (auto *box = dialog->findChild<QDialogButtonBox *>()) {
                    const auto buttons = box->buttons();
                    for (QAbstractButton *button : buttons) {
                        if (box->buttonRole(button) == QDialogButtonBox::AcceptRole) {
                            button->click();
                            return;
                        }
                    }
                }
                QTest::keyClick(dialog, Qt::Key_Return);
            });
            return;
        }
        if (attempts > 0)
            acceptFileDialogLater(path, attempts - 1);
    });
}

// 自动点击消息框上的指定角色按钮
void clickMessageBoxButtonLater(QMessageBox::ButtonRole role)
{
    QTimer::singleShot(100, [role]() {
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
            auto *box = qobject_cast<QMessageBox *>(widget);
            if (!box)
                continue;
            const auto buttons = box->buttons();
            for (QAbstractButton *button : buttons) {
                if (box->buttonRole(button) == role) {
                    button->click();
                    return;
                }
            }
            box->accept();
            return;
        }
    });
}

} // namespace

class TestEditor : public QObject
{
    Q_OBJECT

private slots:
    void initialStatus();
    void statusUpdatesWhileTyping();
    void modifiedTitle();
    void lineNumberToggle();
    void wordWrapToggle();
    void undoRedoActions();
    void copyEnabledWithSelection();
    void saveAndOpenFile();
    void saveAsDialogIntegration();
    void loadNonexistentFile();
    void rejectBinaryFile();
    void findDialogFindsAndWraps();
    void replaceAllCounts();
    void closeCancelKeepsWindow();
    void closeDiscardClosesWindow();
};

void TestEditor::initialStatus()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    QVERIFY(editor);
    QCOMPARE(editor->toPlainText(), QString());
    QVERIFY(window.windowTitle().contains(QStringLiteral("新建文本文件")));
    QVERIFY(!window.isWindowModified());

    QLabel *lengthLabel = statusLabelContaining(window, QStringLiteral("length:"));
    QVERIFY(lengthLabel);
    QVERIFY(lengthLabel->text().contains(QStringLiteral("length: 0")));
    QVERIFY(lengthLabel->text().contains(QStringLiteral("lines: 1")));

    QLabel *cursorLabel = statusLabelContaining(window, QStringLiteral("Ln:"));
    QVERIFY(cursorLabel);
    QVERIFY(cursorLabel->text().contains(QStringLiteral("Ln: 1")));
    QVERIFY(cursorLabel->text().contains(QStringLiteral("Col: 1")));
}

void TestEditor::statusUpdatesWhileTyping()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    editor->setFocus();
    QTest::keyClicks(editor, QStringLiteral("abc"));
    QTest::keyClick(editor, Qt::Key_Return);
    QTest::keyClicks(editor, QStringLiteral("de"));

    QLabel *lengthLabel = statusLabelContaining(window, QStringLiteral("length:"));
    QVERIFY(lengthLabel->text().contains(QStringLiteral("length: 6")));
    QVERIFY(lengthLabel->text().contains(QStringLiteral("lines: 2")));

    QLabel *cursorLabel = statusLabelContaining(window, QStringLiteral("Ln:"));
    QVERIFY(cursorLabel->text().contains(QStringLiteral("Ln: 2")));
    QVERIFY(cursorLabel->text().contains(QStringLiteral("Col: 3")));
}

void TestEditor::modifiedTitle()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    editor->setFocus();
    QVERIFY(!window.isWindowModified());
    QTest::keyClicks(editor, QStringLiteral("x"));
    QVERIFY(window.isWindowModified());

    editor->document()->setModified(false);
    QVERIFY(!window.isWindowModified());
}

void TestEditor::lineNumberToggle()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    auto *action = window.findChild<QAction *>(QStringLiteral("actionShowLineNumber"));
    QVERIFY(editor && action);
    QVERIFY(action->isChecked());
    QVERIFY(editor->lineNumberVisible());

    action->trigger();
    QVERIFY(!editor->lineNumberVisible());
    QCOMPARE(editor->lineNumberAreaWidth(), 0);

    action->trigger();
    QVERIFY(editor->lineNumberVisible());
    QVERIFY(editor->lineNumberAreaWidth() > 0);
}

void TestEditor::wordWrapToggle()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    auto *action = window.findChild<QAction *>(QStringLiteral("actionWordWrap"));
    QVERIFY(editor && action);
    QCOMPARE(editor->lineWrapMode(), QPlainTextEdit::WidgetWidth);

    action->trigger();
    QCOMPARE(editor->lineWrapMode(), QPlainTextEdit::NoWrap);
    action->trigger();
    QCOMPARE(editor->lineWrapMode(), QPlainTextEdit::WidgetWidth);
}

void TestEditor::undoRedoActions()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    auto *undo = window.findChild<QAction *>(QStringLiteral("actionUndo"));
    auto *redo = window.findChild<QAction *>(QStringLiteral("actionRedo"));
    QVERIFY(undo && redo);
    QVERIFY(!undo->isEnabled());
    QVERIFY(!redo->isEnabled());

    editor->setFocus();
    QTest::keyClicks(editor, QStringLiteral("hello"));
    QVERIFY(undo->isEnabled());

    undo->trigger();
    QCOMPARE(editor->toPlainText(), QString());
    QVERIFY(redo->isEnabled());

    redo->trigger();
    QCOMPARE(editor->toPlainText(), QStringLiteral("hello"));
}

void TestEditor::copyEnabledWithSelection()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    auto *copy = window.findChild<QAction *>(QStringLiteral("actionCopy"));
    auto *cut = window.findChild<QAction *>(QStringLiteral("actionCut"));
    QVERIFY(copy && cut);
    QVERIFY(!copy->isEnabled());
    QVERIFY(!cut->isEnabled());

    editor->setPlainText(QStringLiteral("select me"));
    editor->selectAll();
    QVERIFY(copy->isEnabled());
    QVERIFY(cut->isEnabled());

    cut->trigger();
    QCOMPARE(editor->toPlainText(), QString());
    QVERIFY(!copy->isEnabled());
}

void TestEditor::saveAndOpenFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("note.txt"));

    {
        MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
        editor->setPlainText(QStringLiteral("第一行\nsecond line"));
        QVERIFY(window.saveFile(path));
        QVERIFY(window.windowTitle().contains(QStringLiteral("note.txt")));
        QVERIFY(!window.isWindowModified());
    }

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    // Windows 下 QIODevice::Text 会把 \n 写成 \r\n，这里归一化后比较
    content.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    QCOMPARE(content, QStringLiteral("第一行\nsecond line"));

    MainWindow window2;
    window2.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window2));
    auto *editor2 = window2.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    QVERIFY(window2.loadFile(path));
    QCOMPARE(editor2->toPlainText(), QStringLiteral("第一行\nsecond line"));
    QVERIFY(!window2.isWindowModified());
}

void TestEditor::saveAsDialogIntegration()
{
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("saved-as.txt"));

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    editor->setPlainText(QStringLiteral("save as content"));

    acceptFileDialogLater(path);
    window.findChild<QAction *>(QStringLiteral("actionSaveAs"))->trigger();

    QVERIFY(QFile::exists(path));
    QVERIFY(!window.isWindowModified());
    QVERIFY(window.windowTitle().contains(QStringLiteral("saved-as.txt")));
}

void TestEditor::loadNonexistentFile()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    editor->setPlainText(QStringLiteral("keep me"));

    clickMessageBoxButtonLater(QMessageBox::AcceptRole);
    QVERIFY(!window.loadFile(QStringLiteral("Z:/definitely/not/exists/file.txt")));
    QCOMPARE(editor->toPlainText(), QStringLiteral("keep me"));
}

void TestEditor::rejectBinaryFile()
{
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("binary.dat"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QByteArray("\x01\x02\x00\x03binary", 10));
    file.close();

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    editor->setPlainText(QStringLiteral("unchanged"));

    clickMessageBoxButtonLater(QMessageBox::AcceptRole);
    QVERIFY(!window.loadFile(path));
    QCOMPARE(editor->toPlainText(), QStringLiteral("unchanged"));
}

void TestEditor::findDialogFindsAndWraps()
{
    QPlainTextEdit editor;
    editor.setPlainText(QStringLiteral("hello world hello"));

    FindDialog dialog(nullptr, &editor);
    dialog.setSearchText(QStringLiteral("hello"));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));

    auto *findButton = dialog.findChild<QPushButton *>(QStringLiteral("btnFind"));
    QVERIFY(findButton);

    findButton->click();
    QCOMPARE(editor.textCursor().selectedText(), QStringLiteral("hello"));
    QCOMPARE(editor.textCursor().selectionStart(), 0);

    findButton->click();
    QCOMPARE(editor.textCursor().selectionStart(), 12);

    // 第三次应从开头回绕，连续查找
    findButton->click();
    QCOMPARE(editor.textCursor().selectionStart(), 0);
}

void TestEditor::replaceAllCounts()
{
    QPlainTextEdit editor;
    editor.setPlainText(QStringLiteral("a a a"));

    ReplaceDialog dialog(nullptr, &editor);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    dialog.findChild<QLineEdit *>(QStringLiteral("lineEditTarget"))->setText(QStringLiteral("a"));
    dialog.findChild<QLineEdit *>(QStringLiteral("lineEditReplace"))->setText(QStringLiteral("b"));

    clickMessageBoxButtonLater(QMessageBox::AcceptRole);
    dialog.findChild<QPushButton *>(QStringLiteral("btnReplaceAll"))->click();
    QCOMPARE(editor.toPlainText(), QStringLiteral("b b b"));
}

void TestEditor::closeCancelKeepsWindow()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    editor->setPlainText(QStringLiteral("unsaved"));
    editor->document()->setModified(true);

    clickMessageBoxButtonLater(QMessageBox::RejectRole);
    QVERIFY(!window.close());
    QVERIFY(window.isVisible());
}

void TestEditor::closeDiscardClosesWindow()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    editor->setPlainText(QStringLiteral("unsaved"));
    editor->document()->setModified(true);

    clickMessageBoxButtonLater(QMessageBox::DestructiveRole);
    QVERIFY(window.close());
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);
    QApplication app(argc, argv);
    TestEditor test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_editor.moc"
