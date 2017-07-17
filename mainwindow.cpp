#include "mainwindow.h"
#include "ui_mainwindow.h"


Parser::Parser(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

Parser::~Parser()
{
    delete ui;
}

void Parser::open(const QString& filename)
{//open log file and set up new DB if needed
    file = new QFile(filename);
    if (!file->open(QIODevice::ReadOnly))
    {
        qDebug() << "[-]ERROR: Can't open the file";
        exit(-1);
    }

    //set position to 1 because first line in the log file is always empty
    file->seek(1);

    //return from function if dbase already exists
    if (dbase.isOpen())
        return;

    //create new database
    dbase = QSqlDatabase::addDatabase("QSQLITE");
    dbase.setDatabaseName("my_db.sqlite");
    if (!dbase.open())
    {
        qDebug() << "[-]ERROR: Can't open the database";
        exit(-1);
    }

    //create new table in dbase
    QSqlQuery a_query;
    QString str = "CREATE TABLE my_table ("
                  "logID VARCHAR(255), "
                  "tag VARCHAR(255), "
                  "value VARCHAR(255)"
                  ");";
    bool b = a_query.exec(str);
    if (!b)
          qDebug() << "Can't create table/table has already been created";

    return;
}

void Parser::parsFile()
{//separate each log in a file and parse them in PasrMsg()
    QVector<QString>data(0);
    QString line;
    QTextStream in(file);

    //Read until the end of file
    while (!in.atEnd())
    {//if we have empty line, means we reached end of the log
        line = in.readLine();
        if (line != "\0")
        {
            data.push_back(line);
        }
        else
        {
            data.back().append(";");
            parsMsg(data);
            data.clear();
        }
    }
    data.back().append(";");
    parsMsg(data);

    return;
}

void Parser::parsMsg(QVector<QString>& data)
{//separate each part of the log and parse it in parsPart()
    QString generalString = "\0";
    int verboseMode = 0;

    //if verbose mode is set
    if (ui->radioButton->isChecked())
        verboseMode = 1;

    /*data[0] contains first_line and data[1] contains the header
    append ';' because the function that divides log into pairs seeks for ';'
    symbol and if it won't be at the end it will skip the last pair */

    data[0].append(";");
    log.first_line = parsPart(data[0]);
    data[1].append(";");
    log.header	   = parsPart(data[1]);

    /*the rest of the data is filled with varying amount of values
    concatenating them all into the one generalString*/
    for (int i = 2;i < data.size(); i++)
        generalString.append(data[i]);
    log.values = parsPart(generalString);

    if (!dbase.isOpen())
    {
        qDebug() << "[-]ERROR: Can't open the database";
        exit(-1);
    }

    QString query;
    QString logsCount_str;
    QSqlQuery a_query;

    //change some modes to improve perfomance
    dbase.exec("PRAGMA synchronous=OFF");
    dbase.exec("PRAGMA count_changes=OFF");
    dbase.exec("PRAGMA journal_mode=MEMORY");
    dbase.exec("PRAGMA temp_store=MEMORY");
    dbase.exec("BEGIN TRANSACTION");

    logsCount_str = QString::number(logsCount);

    //write into TextEdit
    if (verboseMode)
        ui->textEdit->append("\n==================================\nLog#" + logsCount_str + "\n");

    for (int l = 0; l < log.first_line.size();l++)
    {
        if (verboseMode)
            ui->textEdit->append(log.first_line[l].first + "\t\t\t" + log.first_line[l].second);

        query = QString("INSERT into my_table(logID, tag, value) VALUES ('%1','%2','%3')").arg(logsCount_str).arg(log.first_line[l].first).arg(log.first_line[l].second);
        a_query.prepare(query);
        if (!a_query.exec())
                qDebug() << "[-]ERROR: Can't insert data1";
    }

    for (int l = 0; l < log.header.size();l++)
    {
        if (verboseMode)
            ui->textEdit->append(log.header[l].first + "\t\t\t" + log.header[l].second);

        query = QString("INSERT into my_table(logID, tag, value) values ('%1','%2','%3')").arg(logsCount_str).arg(log.header[l].first).arg(log.header[l].second);
        a_query.prepare(query);
        if (!a_query.exec())
                qDebug() << "[-]ERROR: Can't insert data2";
    }

    for (int l = 0; l < log.values.size();l++)
    {
        if (verboseMode)
            ui->textEdit->append(log.values[l].first + "\t\t\t" + log.values[l].second);
        query = QString("INSERT into my_table(logID, tag, value) VALUES ('%1','%2','%3')").arg(logsCount_str).arg(log.values[l].first).arg(log.values[l].second);
        a_query.prepare(query);
        if (!a_query.exec())
                qDebug() << "[-]ERROR: Can't insert data3";
    }
    dbase.exec("COMMIT TRANSACTION");
    logsCount++;

    return;
}

val Parser::parsPart(QString& data)
{//divides QString data into pairs and stores them in a QVector
    QString part;
    int index = 0;
    QPair<QString,QString> pairOfTagAndValue;
    val vectOfPairs(0);

    //look for the first occurence of ';'
    while((index = data.indexOf(';', 0)) != -1)
    {
        //separate part of line before ';' symbol and write it in to the QString named part
        part = data.mid(0, index);
        data = data.mid(index + 1, data.length() - index);

        if ((index = part.indexOf('=', 0)) != -1)
        {//if there is a '=' symbol means we have tag with value in part QString
            pairOfTagAndValue.first  = part.mid(0, index);
            pairOfTagAndValue.second = part.mid(index + 1, part.length());
        }
        else
        {//no '=' sign means there is only a tag without any value
            pairOfTagAndValue.first  = part;
            pairOfTagAndValue.second = "\0";
        }
        vectOfPairs.push_back(pairOfTagAndValue);
    }

    return vectOfPairs;
}

QString Parser::on_pushButton_clicked()
{
    ui->lineEdit->setText(QFileDialog::getOpenFileName(this, tr("Open text"), "", tr("text Files (*.log)")));

    //name of the log file selected by user
    logFilename = ui->lineEdit->text();
    return logFilename;
}

void Parser::on_pushButton_2_clicked()
{//open log file, parse it, close it
    QTime time;

    qDebug() << "File:" << logFilename << "\n";
    open(logFilename);
    time.start();

    parsFile();
    file->close();

    qDebug() << "END. Time elapsed: "<< time.elapsed() << "ms\n";

    return;
}

void Parser::on_pushButton_4_clicked()
{//close database and exit the program
    dbase.close();
    //dbase.removeDatabase("my_db.sqlite");
    //QSqlDatabase::removeDatabase("my_db.sqlite");
    exit(0);
}
