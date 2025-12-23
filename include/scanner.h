#pragma once

#include <QObject>
#include <QThread>
#include <QDebug>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h> // Для получения имени хоста
#include <cstring>
#include <vector>
#include <map>

struct icmp_packet {
    struct icmphdr header;
    char msg[64];
};

class Scanner : public QObject {
    Q_OBJECT

public:
    explicit Scanner(QObject *parent = nullptr);
    unsigned short checksum(void *b, int len);

public slots:
    void scanRange(QString baseIp);

signals:
    void hostFound(QString ip, QString mac, QString status, QString type);
    void scanFinished();
    void logMessage(QString msg);

private:
    QString getMacAddress(const QString& targetIp);
    QString getHostname(const QString& ip); // Новое: Имя устройства
    QString getVendor(const QString& mac);  // Новое: Производитель
    
    std::vector<int> scanPorts(const QString& ip);
    
    // сигнатура для анализа всех данных
    QString analyzeDevice(const QString& ip, const QString& mac, const std::vector<int>& ports, const QString& hostname);
};