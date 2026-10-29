#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QIcon>
#include <QPushButton>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->labelIcon->setPixmap(QIcon(QStringLiteral(":/icons/app.svg")).pixmap(110, 110));
    connect(ui->btnOK, &QPushButton::clicked, this, &AboutDialog::accept);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
