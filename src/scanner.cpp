#include "scanner.h"
#include <fcntl.h> 
#include <fstream> 
#include <iostream>

using namespace std;

Scanner::Scanner(QObject *parent) : QObject(parent) {}

unsigned short Scanner::checksum(void *b, int len) {
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum = 0;
    unsigned short result;
    for (sum = 0; len > 1; len -= 2) sum += *buf++;
    if (len == 1) sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

QString Scanner::getMacAddress(const QString& targetIp) {
    ifstream arpFile("/proc/net/arp");
    string line;
    if (!arpFile.is_open()) return "00:00:00:00:00:00";
    getline(arpFile, line);
    while (getline(arpFile, line)) {
        if (line.find(targetIp.toStdString()) != string::npos) {
            size_t firstColon = line.find(':');
            if (firstColon != string::npos && firstColon >= 2) {
                return QString::fromStdString(line.substr(firstColon - 2, 17));
            }
        }
    }
    return "Не определен";
}

// получение имени хоста (reverse DNS)
QString Scanner::getHostname(const QString& ip) {
    struct sockaddr_in sa;
    char host[1024];
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, ip.toStdString().c_str(), &sa.sin_addr);

    if (getnameinfo((struct sockaddr*)&sa, sizeof(sa), host, sizeof(host), NULL, 0, NI_NAMEREQD) == 0) {
        return QString::fromUtf8(host);
    }
    return "";
}

// определение производителя по мак адресу (OUI)
QString Scanner::getVendor(const QString& mac) {
    QString prefix = mac.left(8).toUpper();
    if (prefix.startsWith("00:17:F2") || prefix.startsWith("00:25:00") || prefix.startsWith("F0:99:B6") || prefix.startsWith("40:99:99")) return "Apple";
    if (prefix.startsWith("00:12:47") || prefix.startsWith("34:14:5F") || prefix.startsWith("38:01:97")) return "Samsung";
    if (prefix.startsWith("00:1D:7E") || prefix.startsWith("B8:A3:86") || prefix.startsWith("C0:4A:00")) return "TP-Link";
    if (prefix.startsWith("04:D9:F5") || prefix.startsWith("50:46:5D")) return "Asus";
    if (prefix.startsWith("E8:94:F6")) return "Keenetic";
    if (prefix.startsWith("00:1B:21") || prefix.startsWith("84:7B:EB")) return "Intel";
    if (prefix.startsWith("00:D8:61")) return "MSI";
    if (prefix.startsWith("08:00:27")) return "VirtualBox";
    if (prefix.startsWith("00:0C:29") || prefix.startsWith("00:50:56")) return "VMware";
    return ""; 
}

vector<int> Scanner::scanPorts(const QString& ip) {
    vector<int> openPorts;
    vector<int> targets = {21, 22, 53, 80, 443, 445, 3389, 5000, 5555, 62078}; 

    for (int port : targets) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.toStdString().c_str(), &addr.sin_addr);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 150000; 
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);

        if (::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            openPorts.push_back(port);
        }
        close(sock);
    }
    return openPorts;
}

QString Scanner::analyzeDevice(const QString& ip, const QString& mac, const vector<int>& ports, const QString& hostname) {
    bool pWindows = false, pLinux = false, pApple = false, pAndroid = false, pWeb = false, pDNS = false;
    for(int p : ports) {
        if(p == 135 || p == 139 || p == 445 || p == 3389) pWindows = true;
        if(p == 22) pLinux = true;
        if(p == 62078) pApple = true;
        if(p == 5555) pAndroid = true;
        if(p == 80 || p == 443) pWeb = true;
        if(p == 53) pDNS = true;
    }

    QString vendor = getVendor(mac);
    QString lowerHost = hostname.toLower();

    if (pApple) return "Apple Device (iOS)";
    if (pAndroid) return "Android Device";
    if (pWindows) return "Windows PC";
    if (lowerHost.contains("android")) return "Android Device";
    if (lowerHost.contains("iphone") || lowerHost.contains("ipad") || lowerHost.contains("macbook")) return "Apple Device";
    if (lowerHost.contains("desktop") || lowerHost.contains("laptop")) return "Windows PC";
    if (ip.endsWith(".1")) return "Router/Gateway";
    if (pLinux) return "Linux Server";
    if (pWeb && !pWindows && !pLinux) return "IoT/Smart Device";
    if (vendor == "Apple") return "Apple Device";
    if (vendor == "Samsung") return "Mobile/Tablet";
    if (vendor == "Intel") return "PC/Laptop";
    if (vendor == "TP-Link" || vendor == "Keenetic" || vendor == "Asus") return "Router/AP";
    if (ports.empty()) return "Mobile/Tablet (Sleep)";
    return "Unknown Device";
}

void Scanner::scanRange(QString inputIp) {
    if (inputIp.startsWith("127.")) {
        emit logMessage("ОШИБКА: 127.0.0.1 - это локальный интерфейс (Loopback).");
        emit scanFinished();
        return;
    }

    QStringList parts = inputIp.split('.');
    if (parts.size() < 3) {
        emit logMessage("ОШИБКА: Некорректный формат IP адреса.");
        emit scanFinished();
        return;
    }

    QString baseIp = parts[0] + "." + parts[1] + "." + parts[2];
    emit logMessage(">>> Анализ подсети: " + baseIp + ".0/24");
    
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) { 
        emit logMessage("ROOT ERROR: Нужны права суперпользователя (sudo)!");
        emit scanFinished(); 
        return; 
    }

    struct timeval tv;
    tv.tv_sec = 0; tv.tv_usec = 200000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    for (int i = 1; i < 255; i++) {
        QString currentIpStr = baseIp + "." + QString::number(i);
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, currentIpStr.toStdString().c_str(), &addr.sin_addr);

        struct icmp_packet packet;
        memset(&packet, 0, sizeof(packet));
        packet.header.type = ICMP_ECHO;
        packet.header.code = 0;
        packet.header.un.echo.id = getpid();
        packet.header.un.echo.sequence = i;
        packet.header.checksum = checksum(&packet, sizeof(packet));

        if (sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(addr)) > 0) {
            char buf[1024];
            struct sockaddr_in r_addr;
            socklen_t addr_len = sizeof(r_addr);
            
            if (recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&r_addr, &addr_len) > 0) {
                struct iphdr *ipHeader = (struct iphdr *)buf;
                struct icmphdr *icmpHeader = (struct icmphdr *)(buf + (ipHeader->ihl * 4));
                
                if (icmpHeader->type == ICMP_ECHOREPLY) {
                    char sender[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(r_addr.sin_addr), sender, INET_ADDRSTRLEN);
                    QString foundIp = QString::fromUtf8(sender);
                    
                    if (foundIp == currentIpStr) {
                        QString mac = getMacAddress(foundIp);
                        vector<int> ports = scanPorts(foundIp);
                        QString hostname = getHostname(foundIp);
                        QString type = analyzeDevice(foundIp, mac, ports, hostname);
                        emit hostFound(foundIp, mac, "Online", type);
                    }
                }
            }
        }
        QThread::usleep(3000); 
    }
    close(sock);
    emit scanFinished();
}