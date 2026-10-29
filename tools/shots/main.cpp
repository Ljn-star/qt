#include "mainwindow.h"
#include "aboutdialog.h"
#include "finddialog.h"
#include "replacedialog.h"
#include "codeeditor.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QEventLoop>
#include <QFont>
#include <QLineEdit>
#include <QMenu>
#include <QTimer>

static void waitFor(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setFont(QFont(QStringLiteral("Microsoft YaHei UI"), 10));

    const QString outDir = (argc > 1) ? QString::fromLocal8Bit(argv[1])
                                      : QStringLiteral("shots-out");
    QDir().mkpath(outDir);

    MainWindow window;
    window.resize(1000, 680);
    window.show();
    waitFor(400);

    auto *editor = window.findChild<CodeEditor *>(QStringLiteral("textEdit"));
    editor->setPlainText(QStringLiteral(
        "// 小牛文本编辑器 —— 实验2\n"
        "#include <QApplication>\n"
        "#include <QMainWindow>\n"
        "\n"
        "int main(int argc, char *argv[])\n"
        "{\n"
        "    QApplication app(argc, argv);\n"
        "\n"
        "    // 支持行号、查找替换、字体颜色等功能\n"
        "    return app.exec();\n"
        "}\n"));
    editor->document()->setModified(false);
    waitFor(300);
    window.grab().save(outDir + QStringLiteral("/main-window.png"));

    // cut / copy / paste 使能状态对比：编辑菜单
    auto *menuEdit = window.findChild<QMenu *>(QStringLiteral("menuEdit"));
    if (menuEdit) {
        QApplication::clipboard()->clear();
        QTextCursor cursor = editor->textCursor();
        cursor.clearSelection();
        cursor.movePosition(QTextCursor::Start);
        editor->setTextCursor(cursor);
        waitFor(200);

        const QPoint pos = window.mapToGlobal(QPoint(8, 62));
        menuEdit->move(pos);
        menuEdit->show();
        waitFor(500);
        const QPixmap shot1 = menuEdit->grab();
        shot1.save(outDir + QStringLiteral("/edit-menu-disabled.png"));
        menuEdit->hide();
        waitFor(200);

        editor->selectAll();
        QApplication::clipboard()->setText(QStringLiteral("clipboard"));
        waitFor(300);

        menuEdit->move(pos);
        menuEdit->show();
        waitFor(500);
        const QPixmap shot2 = menuEdit->grab();
        shot2.save(outDir + QStringLiteral("/edit-menu-enabled.png"));
        menuEdit->hide();
    }

    {
        AboutDialog dialog(&window);
        dialog.show();
        waitFor(300);
        dialog.grab().save(outDir + QStringLiteral("/about-dialog.png"));
    }

    {
        FindDialog dialog(&window, editor);
        dialog.setSearchText(QStringLiteral("app"));
        dialog.show();
        waitFor(300);
        dialog.grab().save(outDir + QStringLiteral("/find-dialog.png"));
    }

    {
        ReplaceDialog dialog(&window, editor);
        dialog.findChild<QLineEdit *>(QStringLiteral("lineEditTarget"))
            ->setText(QStringLiteral("app"));
        dialog.findChild<QLineEdit *>(QStringLiteral("lineEditReplace"))
            ->setText(QStringLiteral("application"));
        dialog.show();
        waitFor(300);
        dialog.grab().save(outDir + QStringLiteral("/replace-dialog.png"));
    }

    return 0;
}
