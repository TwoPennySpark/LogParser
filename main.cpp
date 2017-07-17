#include "mainwindow.h"
#include <QApplication>
#include <QtSql>
#include "ui_mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Parser message;

    message.show();
    return a.exec();

}
