#include "dbmanager.h"
#include "db_schema.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QCoreApplication>

DbManager::DbManager() {
}

DbConfig DbManager::loadConfig() {
    DbConfig config;
    
    // значения по умолчанию
    config.host = "127.0.0.1";
    config.name = "network_scan";
    config.user = "postgres";
    config.password = "";
    config.port = 5432;

    // поиск файла .env рядом с исполняемым
    QString configPath = QCoreApplication::applicationDirPath() + "/.env";
    QFile file(configPath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("#") || line.isEmpty()) continue;

            QStringList parts = line.split("=");
            if (parts.size() >= 2) {
                QString key = parts[0].trimmed();
                QString value = parts[1].trimmed();

                if (key == "DB_HOST") config.host = value;
                else if (key == "DB_NAME") config.name = value;
                else if (key == "DB_USER") config.user = value;
                else if (key == "DB_PASSWORD") config.password = value;
                else if (key == "DB_PORT") config.port = value.toInt();
            }
        }
        file.close();
        qDebug() << "Конфигурация загружена из:" << configPath;
    } else {
        qDebug() << "ПРЕДУПРЕЖДЕНИЕ: Файл .env не найден! Используются настройки по умолчанию.";
    }

    return config;
}

bool DbManager::connectToDb() {
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QPSQL");
    }
    
    // загрузка конфига
    DbConfig config = loadConfig();

    db.setHostName(config.host);             
    db.setDatabaseName(config.name);      
    db.setUserName(config.user);                  
    db.setPassword(config.password);              
    db.setPort(config.port);

    if (!db.open()) {
        qDebug() << "Ошибка подключения к БД:" << db.lastError().text();
        return false;
    }
    return true;
}

bool DbManager::initTables() {
    QSqlQuery query;
    
    QStringList scripts = {
        SQL_CREATE_TABLES,   
        SQL_FUNC_ADD_HOST,   
        SQL_FUNC_GET_HOSTS,  
        SQL_FUNC_CLEAR,
        
        SQL_CREATE_VENDORS,  
        SQL_SEED_VENDORS,    
        SQL_FUNC_GET_VENDOR  
    };

    for (const QString& sql : scripts) {
        if (!query.exec(sql)) {
            qDebug() << "Init DB Error:" << query.lastError().text();
        }
    }
    return true;
}

bool DbManager::addHost(const QString& ip, const QString& mac, const QString& type) {
    QSqlQuery query;
    
    query.prepare("SELECT sp_add_host(:ip, :mac, :type)");
    
    query.bindValue(":ip", ip);
    query.bindValue(":mac", mac);
    query.bindValue(":type", type);

    if (!query.exec()) {
        qDebug() << "Add Host Error:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<QStringList> DbManager::getAllHosts() {
    QList<QStringList> list;
    
    QSqlQuery query("SELECT * FROM sp_get_all_hosts()");
    
    while (query.next()) {
        QStringList host;
        host << query.value(0).toString(); // ip
        host << query.value(1).toString(); // mac
        host << "History";                 // status
        host << query.value(2).toString(); // type
        list.append(host);
    }
    return list;
}

bool DbManager::clearHosts() {
    QSqlQuery query;
    if(query.exec("SELECT sp_clear_hosts()")) {
        return true;
    } else {
        qDebug() << "Clear Error:" << query.lastError().text();
        return false;
    }
}

QString DbManager::getVendorByMac(QString mac) {
    QSqlQuery query;
    query.prepare("SELECT sp_get_vendor(:mac)");
    query.bindValue(":mac", mac);
    
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "Unknown";
}