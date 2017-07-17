#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QPair>
#include <QFile>
#include <QDebug>
#include <QtSql>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <iterator>
#include <cstdio>
#include <QFileDialog>
#include "ui_mainwindow.h"

typedef QVector<QPair<QString, QString>> val;

namespace Ui
{
    class MainWindow;
}

typedef struct logStructure
{
    val first_line;
    val header;
    val values;
}logStruct;


class Parser : public QMainWindow
{
    Q_OBJECT
private:
    int logsCount = 1;
    logStruct log;
    QFile *file;
    QString logFilename;
    QSqlDatabase dbase;
    Ui::MainWindow *ui;
public:
    explicit Parser(QWidget *parent = 0);
    ~Parser();
    void open(const QString& filename);
    void parsMsg(QVector<QString>& data);
    void parsFile();
    val  parsPart(QString& str);
    void close();
private slots:
    QString on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_4_clicked();
};

#endif // MAINWINDOW_H
