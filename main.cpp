#include "mainwindow.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QtSql>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
       MainWindow w;
       w.show();

       QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
       db.setDatabaseName("saved_files");
       if (!db.open()) {
           qDebug() << "Failed to open database";
       }

       QSqlQuery query1;
       query1.exec("CREATE TABLE saved_files (file_id INTEGER PRIMARY KEY AUTOINCREMENT, file_name VARCHAR, time DATE, file_size VARCHAR)");

       QSqlDatabase templates_db = QSqlDatabase::addDatabase("QSQLITE", "templates_db");
       templates_db.setDatabaseName("saved_templates");
       if (!templates_db.open()) {
           qDebug() << "Failed to open templates database";
       }

       QSqlQuery query3;
       if(query3.exec("CREATE TABLE file_groups (group_id INTEGER PRIMARY KEY AUTOINCREMENT, file_string VARCHAR)"))
       {
            qDebug() << "Query executed\n";
       }

       QSqlQuery query2(templates_db);
       query2.exec("CREATE TABLE saved_templates (template_id INTEGER PRIMARY KEY AUTOINCREMENT, template_name TEXT,time DATE , file_paths TEXT , scan_type INTEGER DEFAULT 0, scan_hour TEXT, scan_minute TEXT, weekly_day INTEGER DEFAULT 0, monthly_day INTEGER DEFAULT 0)");

       return a.exec();
}
