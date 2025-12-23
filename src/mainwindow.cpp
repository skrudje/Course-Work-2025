#include "mainwindow.h"
#include "stylehelper.h"
#include <QHeaderView>
#include <cmath>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QMessageBox> 
#include <QMenuBar>
#include <QMenu>
#include <QAction>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    setWindowTitle("NetScan v1.0"); 
    resize(1150, 800);

    setupMenu();

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 5, 0, 0);
    mainLayout->setSpacing(5);

    // header
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(10, 5, 10, 5);

    targetLabel = new QLabel("Цель:", this);
    targetLabel->setStyleSheet(StyleHelper::getLabelStyle()); 
    headerLayout->addWidget(targetLabel);

    ipInput = new QLineEdit("192.168.0.", this);
    ipInput->setFixedWidth(180);
    ipInput->setStyleSheet(StyleHelper::getLineEditStyle(isDarkTheme));
    headerLayout->addWidget(ipInput);

    scanButton = new QPushButton("Начать анализ", this);
    scanButton->setCursor(Qt::PointingHandCursor);
    scanButton->setMinimumHeight(36);
    scanButton->setFixedWidth(150);
    scanButton->setStyleSheet(StyleHelper::getStartButtonStyle());
    headerLayout->addWidget(scanButton);
    headerLayout->addStretch(); 

    mainLayout->addLayout(headerLayout);

    // tabs
    QTabWidget* tabs = new QTabWidget(this);
    tabs->setStyleSheet(StyleHelper::getTabStyle(isDarkTheme));
    mainLayout->addWidget(tabs);

    // tab 1: cписок
    QWidget* listTab = new QWidget();
    QVBoxLayout* listLayout = new QVBoxLayout(listTab);
    listLayout->setContentsMargins(0,0,0,0);

    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("🔍 Поиск по таблице (IP, MAC, Тип)...");
    searchBar->setStyleSheet(StyleHelper::getLineEditStyle(isDarkTheme));
    listLayout->addWidget(searchBar);

    connect(searchBar, &QLineEdit::textChanged, [=](const QString &text){
        for(int i = 0; i < resultTable->rowCount(); ++i) {
            bool match = false;
            for(int j = 0; j < resultTable->columnCount(); ++j) {
                QTableWidgetItem *item = resultTable->item(i, j);
                if(item && item->text().contains(text, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
            resultTable->setRowHidden(i, !match);
        }
    });
    
    resultTable = new QTableWidget(this);
    resultTable->setColumnCount(4);
    resultTable->setHorizontalHeaderLabels({"IP Адрес", "MAC Адрес", "Статус", "Тип устройства"});
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers); 
    resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultTable->setAlternatingRowColors(true);
    resultTable->verticalHeader()->setVisible(false);
    listLayout->addWidget(resultTable);
    tabs->addTab(listTab, "📋 Список узлов");

    resultTable->setSortingEnabled(true);

    // tab 2: Топология
    topologyWidget = new TopologyWidget(this);
    tabs->addTab(topologyWidget, "🕸️ Топология");

    // tab 3: LOGS
    QWidget* logTab = new QWidget();
    QVBoxLayout* logLayout = new QVBoxLayout(logTab);
    logLayout->setContentsMargins(0,0,0,0);
    
    logConsole = new QPlainTextEdit(this);
    logConsole->setReadOnly(true);
    logLayout->addWidget(logConsole);
    tabs->addTab(logTab, "💻 Журнал");

    // status bar
    QWidget* statusBar = new QWidget(this);
    statusBar->setFixedHeight(35);
    statusBar->setStyleSheet(StyleHelper::getStatusBarStyle());
    
    QHBoxLayout* statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(10, 0, 10, 0);
    
    statusLabel = new QLabel("Готов.", this);
    progressBar = new QProgressBar(this); 
    progressBar->setRange(0, 254);
    progressBar->setValue(0);
    progressBar->setFixedWidth(150);
    progressBar->setTextVisible(false);
    progressBar->setStyleSheet(StyleHelper::getProgressBarStyle());
    
    foundLabel = new QLabel("Найдено: 0", this);

    statusLayout->addWidget(statusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(progressBar);
    statusLayout->addWidget(foundLabel);

    mainLayout->addWidget(statusBar); 

    // логика
    connect(scanButton, &QPushButton::clicked, this, &MainWindow::onScanClicked);
    
    // бд
    if (dbManager.connectToDb()) {
        dbManager.initTables();
        
        // загрузка данных с бд
        QList<QStringList> history = dbManager.getAllHosts();
        for (const QStringList& host : history) {
            int row = resultTable->rowCount();
            resultTable->insertRow(row);
            resultTable->setItem(row, 0, new QTableWidgetItem(host[0]));
            resultTable->setItem(row, 1, new QTableWidgetItem(host[1]));
            
            QTableWidgetItem* statusItem = new QTableWidgetItem(host[2]);
            statusItem->setForeground(QBrush(Qt::gray));
            resultTable->setItem(row, 2, statusItem);
            
            resultTable->setItem(row, 3, new QTableWidgetItem(host[3]));
            
            // добавка на карту топологии
            topologyWidget->addDevice(host[0], row, host[3]);
        }
        addLog("Загружено из базы данных: " + QString::number(history.size()) + " узлов.");
    }

    // потоки
    scanThread = new QThread(this);
    scanner = new Scanner();
    scanner->moveToThread(scanThread);

    connect(scanThread, &QThread::started, [=](){
        scanner->scanRange(ipInput->text()); 
    });

    // обработка результатов
    connect(scanner, &Scanner::hostFound, this, [=](QString ip, QString mac, QString status, QString type){
        int row = resultTable->rowCount();
        resultTable->insertRow(row);

        QString vendor = dbManager.getVendorByMac(mac);

        if (type == "Unknown Device" || type.isEmpty()) {
            if (vendor != "Unknown") {
                if (vendor == "Apple") type = "Apple Device";
                else if (vendor == "Keenetic" || vendor == "TP-Link") type = "Router";
                else type = vendor + " Device";
            }
        }
        
        resultTable->setItem(row, 0, new QTableWidgetItem(ip));
        resultTable->setItem(row, 1, new QTableWidgetItem(mac));
        
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);
        statusItem->setForeground(QBrush(QColor(0, 230, 118))); 
        statusItem->setFont(QFont("Arial", 9, QFont::Bold));
        resultTable->setItem(row, 2, statusItem);
        
        resultTable->setItem(row, 3, new QTableWidgetItem(type));
        
        dbManager.addHost(ip, mac, type); 
        
        topologyWidget->addDevice(ip, row, type); 
        
        foundLabel->setText("Найдено: " + QString::number(row + 1));
        addLog("[+] НАЙДЕН: " + ip + " (" + type + ")");
    });

    connect(scanner, &Scanner::logMessage, this, &MainWindow::addLog);

    connect(scanner, &Scanner::scanFinished, this, [=](){
        statusLabel->setText("Готово.");
        progressBar->setValue(254); 
        scanButton->setText("Анализ трафика");
        scanButton->setStyleSheet(StyleHelper::getStartButtonStyle()); 
        scanButton->setEnabled(true);
        addLog("--- Сканирование завершено ---");
        scanThread->quit();
    });

    applyThemeColors();
    addLog("Система готова.");
}

MainWindow::~MainWindow() {
    if(scanThread->isRunning()) {
        scanThread->quit();
        scanThread->wait();
    }
}

void MainWindow::onScanClicked() {
    QString target = ipInput->text();
    if (target.length() < 7 || target.count('.') < 1) {
        QMessageBox::warning(this, "Ошибка ввода", "Введите корректный IP (например 192.168.0.1)");
        return;
    resultTable->setRowCount(0);
    topologyWidget->clearMap();
    logConsole->clear();
    }

    resultTable->setRowCount(0);
    progressBar->setValue(0);
    foundLabel->setText("Найдено: 0");
    logConsole->clear();

    topologyWidget->clearMap();
    topologyWidget->setCenterNode();

    statusLabel->setText("Сканирование сети...");
    scanButton->setText("Анализ...");
    scanButton->setStyleSheet(StyleHelper::getStopButtonStyle()); 
    scanButton->setEnabled(false); 
    
    
    scanThread->start(); 
}

void MainWindow::applyThemeColors() {
    QPalette p = qApp->palette();
    
    resultTable->setStyleSheet(StyleHelper::getTableStyle(isDarkTheme));
    logConsole->setStyleSheet(StyleHelper::getLogStyle(isDarkTheme));
    ipInput->setStyleSheet(StyleHelper::getLineEditStyle(isDarkTheme));
    searchBar->setStyleSheet(StyleHelper::getLineEditStyle(isDarkTheme));

    QTabWidget* tabs = centralWidget->findChild<QTabWidget*>();
    if (tabs) {
        tabs->setStyleSheet(StyleHelper::getTabStyle(isDarkTheme));
    }

    topologyWidget->updateTheme(isDarkTheme);

    if (isDarkTheme) {
        p.setColor(QPalette::Window, QColor(53, 53, 53));
        p.setColor(QPalette::WindowText, Qt::white);
        p.setColor(QPalette::Base, QColor(35, 35, 35));
        p.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        p.setColor(QPalette::Text, Qt::white);
        p.setColor(QPalette::Button, QColor(53, 53, 53));
        p.setColor(QPalette::ButtonText, Qt::white);
        targetLabel->setStyleSheet("color: white; font-weight: bold; font-size: 14px;");
    } else {
        p.setColor(QPalette::Window, QColor(245, 245, 245));
        p.setColor(QPalette::WindowText, Qt::black);
        p.setColor(QPalette::Base, Qt::white);
        p.setColor(QPalette::Text, Qt::black);
        p.setColor(QPalette::Button, QColor(240, 240, 240));
        p.setColor(QPalette::ButtonText, Qt::black);
        targetLabel->setStyleSheet("color: black; font-weight: bold; font-size: 14px;");
    }
    
    qApp->setPalette(p);
}

// остальные методы (меню, контакты, экспорт и т.д.)
void MainWindow::setupMenu() {
    QMenuBar* menu = menuBar();
    
    QMenu* fileMenu = menu->addMenu("Файл");
    connect(fileMenu->addAction("Экспорт в CSV"), &QAction::triggered, this, &MainWindow::onExportClicked);
    connect(fileMenu->addAction("Очистить базу"), &QAction::triggered, this, &MainWindow::onClearClicked);
    fileMenu->addSeparator();
    connect(fileMenu->addAction("Выход"), &QAction::triggered, this, &QMainWindow::close);
    
    QMenu* viewMenu = menu->addMenu("Вид");
    connect(viewMenu->addAction("🌗 Сменить тему"), &QAction::triggered, this, &MainWindow::toggleTheme);
    
    QMenu* helpMenu = menu->addMenu("Помощь");
    connect(helpMenu->addAction("Руководство пользователя"), &QAction::triggered, this, &MainWindow::showContacts);
    
    menu->addAction("О программе", this, &MainWindow::showAbout);
}

void MainWindow::addLog(const QString& message) {
    QString time = QDateTime::currentDateTime().toString("HH:mm:ss");
    logConsole->appendPlainText("[" + time + "] " + message);
    if(message.contains("START")) progressBar->setValue(0);
}

void MainWindow::toggleTheme() {
    isDarkTheme = !isDarkTheme;
    applyThemeColors();
}

void MainWindow::showContacts() {
    QMessageBox::information(this, "Руководство пользователя",
        "<h3> Быстрый старт</h3>"
        "<p><b>1. Сканирование:</b> Введите базовый IP сети (например, <i>192.168.0.</i> или <i>192.168.0.1</i>) и нажмите кнопку <b>АНАЛИЗ</b>.</p>"
        "<p><b>2. Список узлов:</b> Найденные устройства отобразятся в таблице.</p>"
        "<ul>"
        "<li><span style='color:green;'><b>Зеленый статус</b></span>: Устройство активно прямо сейчас.</li>"
        "<li><span style='color:gray;'><b>Серый статус (History)</b></span>: Устройство было в сети ранее (загружено из БД).</li>"
        "</ul>"
        "<p><b>3. Топология:</b> Перейдите на вкладку 'Топология', чтобы увидеть визуальную карту сети.</p>"
        "<p><b>4. Поиск:</b> Используйте строку поиска для фильтрации по IP, MAC или Вендору.</p>"
        "<hr>"
        "<p><i>Разработано в рамках курсового проекта (2025).</i></p>"
    );
}

void MainWindow::showAbout() {
    QMessageBox::about(this, "О программе NetScan",
        "<h2 style='color:#2980b9;'>NetScan v1.0</h2>"
        "<p><b>Сетевой сканер и анализатор топологии локальной сети.</b></p>"
        "<p>Программа предназначена для обнаружения активных хостов, определения их типов (OS Fingerprinting) и визуализации структуры сети.</p>"
        "<hr>"
        "<b>Технический стек:</b>"
        "<ul>"
        "<li><b>Язык:</b> C++ / Qt 5</li>"
        "<li><b>Сеть:</b> Raw Sockets (ICMP/TCP), ARP request analysis</li>"
        "<li><b>База данных:</b> PostgreSQL 15 (Docker Container)</li>"
        "<li><b>Архитектура:</b> Multithreading (QThread)</li>"
        "</ul>"
        "<hr>"
        "<p><b>Разработчик:</b> Andronov Andrey<br>"
        "<b>Email:</b> skrudjbz@gmail.com</p>"
        "<p><i>Новосибирский государственный технический университет (НГТУ)</i></p>"
    );
}

void MainWindow::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить", "Report.csv", "CSV (*.csv)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "IP,MAC,Status,Type\n";
        for (int i = 0; i < resultTable->rowCount(); ++i) {
             out << resultTable->item(i,0)->text() << "," << resultTable->item(i,1)->text() << "," 
                 << resultTable->item(i,2)->text() << "," << resultTable->item(i,3)->text() << "\n";
        }
        file.close();
        addLog("Экспорт завершен.");
    }
}

void MainWindow::onClearClicked() {
    if (QMessageBox::question(this, "Очистка", "Удалить все?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        dbManager.clearHosts();
        resultTable->setRowCount(0);
        logConsole->clear();
        
        topologyWidget->clearMap();
        topologyWidget->setCenterNode();
        
        statusLabel->setText("Готов.");

        progressBar->setValue(0);
        foundLabel->setText("Найдено: 0");
        statusLabel->setText("База очищена.");
        
        addLog("База данных и интерфейс успешно очищены.");
    }
}