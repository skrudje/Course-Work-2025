#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QTabWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QMessageBox>
#include <QApplication>
#include <QStyleFactory>
#include <QProgressBar>
#include <QDateTime>

#include "dbmanager.h"
#include <QThread>
#include "scanner.h"
#include "topologywidget.h" 

using namespace std;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onScanClicked();
    void showContacts();
    void showAbout();
    void toggleTheme();
    void onExportClicked();
    void onClearClicked();
    void addLog(const QString& message); 

private:
    QWidget *centralWidget;
    QLineEdit *ipInput;
    QLineEdit *searchBar;
    QPushButton *scanButton;
    QTableWidget *resultTable;
    
    QLabel *statusLabel;
    QProgressBar *progressBar;
    QLabel *foundLabel;
    QLabel *targetLabel;
    
    QPlainTextEdit *logConsole;

    TopologyWidget *topologyWidget; 

    DbManager dbManager;
    QThread* scanThread;
    Scanner* scanner;

    bool isDarkTheme = false;

    void applyThemeColors();
    void setupMenu();
};