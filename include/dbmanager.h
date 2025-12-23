#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>
#include <QFile>
#include <QTextStream>

using namespace std;

struct DbConfig {
    QString host;
    QString name;
    QString user;
    QString password;
    int port;
};

class DbManager {
public:
    DbManager();
    bool connectToDb();
    bool initTables();
    bool addHost(const QString& ip, const QString& mac, const QString& type);
    QList<QStringList> getAllHosts();
    bool clearHosts();

    QString getVendorByMac(QString mac);

private:
    QSqlDatabase db;

    DbConfig loadConfig();
};
