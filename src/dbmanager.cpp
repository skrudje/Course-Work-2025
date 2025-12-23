#include "dbmanager.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

DbManager::DbManager() {
    // Конструктор пустой, инициализация драйвера в connectToDb
}

bool DbManager::connectToDb() {
    // Проверка, чтобы не создавать подключение дважды
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QPSQL");
    }
    
    db.setHostName("127.0.0.1");             
    db.setDatabaseName("network_scan");      
    db.setUserName("user");                  
    db.setPassword("password");              
    db.setPort(5432);

    if (!db.open()) {
        qDebug() << "Ошибка подключения к БД:" << db.lastError().text();
        return false;
    }
    return true;
}

bool DbManager::initTables() {
    QSqlQuery query;
    // создаем таблицу 
    // важно что если ip TEXT PRIMARY KEY означает, что IP должен быть уникальным
    // если мы найдем этот IP снова, мы просто обновим данные, а не создадим дубликат
    QString createTable = "CREATE TABLE IF NOT EXISTS hosts ("
                          "ip TEXT PRIMARY KEY, "
                          "mac TEXT, "
                          "type TEXT, "
                          "scan_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                          ")";
    
    if (!query.exec(createTable)) {
        qDebug() << "Ошибка создания таблицы:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DbManager::addHost(const QString& ip, const QString& mac, const QString& type) {
    QSqlQuery query;
    
    // upsert запрос: вставить, а если конфликт по IP (такой уже есть) — обновить поля
    query.prepare("INSERT INTO hosts (ip, mac, type, scan_date) "
                  "VALUES (:ip, :mac, :type, CURRENT_TIMESTAMP) "
                  "ON CONFLICT (ip) DO UPDATE SET "
                  "mac = EXCLUDED.mac, "
                  "type = EXCLUDED.type, "
                  "scan_date = CURRENT_TIMESTAMP");
                  
    query.bindValue(":ip", ip);
    query.bindValue(":mac", mac);
    query.bindValue(":type", type);

    if (!query.exec()) {
        qDebug() << "Ошибка записи в БД (" << ip << "):" << query.lastError().text();
        return false;
    }
    return true;
}

QList<QStringList> DbManager::getAllHosts() {
    QList<QStringList> list;
    QSqlQuery query("SELECT ip, mac, type FROM hosts ORDER BY scan_date DESC");
    
    while (query.next()) {
        QStringList host;
        host << query.value(0).toString(); // IP
        host << query.value(1).toString(); // MAC
        host << "History";                 // Статус (так как это из истории)
        host << query.value(2).toString(); // Type
        list.append(host);
    }
    return list;
}

bool DbManager::clearHosts() {
    QSqlQuery query;
    if(query.exec("TRUNCATE TABLE hosts")) {
        return true;
    } else {
        qDebug() << "Ошибка очистки:" << query.lastError().text();
        return false;
    }
}