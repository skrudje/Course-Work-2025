#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

using namespace std;

class DbManager {
public:
    DbManager();
    bool connectToDb();
    bool initTables();
    bool addHost(const QString& ip, const QString& mac, const QString& type);
    QList<QStringList> getAllHosts();
    
    // Очистка таблицы
    bool clearHosts(); 

private:
    QSqlDatabase db;
};
