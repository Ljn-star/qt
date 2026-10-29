#include "mainwindow.h"

#include <QApplication>
#include <QFont>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("SimpleTextEditor"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));
    app.setFont(QFont(QStringLiteral("Microsoft YaHei UI"), 10));

    MainWindow window;
    window.show();

    return app.exec();
}
