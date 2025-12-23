#include "stylehelper.h"

QString StyleHelper::getMainWindowStyle() {
    return ""; // Используем стандартный Fusion, но можно добавить глобальные стили
}

QString StyleHelper::getLabelStyle() {
    return "QLabel { font-weight: bold; font-size: 14px; }";
}

QString StyleHelper::getLineEditStyle(bool isDark) {
    if (isDark) {
        return "QLineEdit { "
               "   background-color: #333; " // Темно-серый фон
               "   color: white; "           // Белый текст
               "   border: 1px solid #555; "
               "   padding: 6px; "
               "   border-radius: 4px; "
               "}";
    } else {
        return "QLineEdit { "
               "   background-color: #fff; " // Белый фон
               "   color: #000; "            // Черный текст
               "   padding: 6px; "
               "   border-radius: 4px; "
               "   border: 1px solid #aaa; "
               "}";
    }
}


QString StyleHelper::getButtonStyle() {
    return "QPushButton { "
           "   background-color: #f0f0f0; "
           "   border: 1px solid #ccc; "
           "   border-radius: 4px; "
           "   padding: 6px 12px; "
           "   color: black; "
           "}"
           "QPushButton:hover { background-color: #e0e0e0; }";
}

QString StyleHelper::getStartButtonStyle() {
    return "QPushButton { "
           "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9); "
           "   color: white; "
           "   border-radius: 4px; "
           "   font-weight: bold; "
           "   border: 1px solid #2980b9; "
           "} "
           "QPushButton:hover { background: #3498db; } "
           "QPushButton:pressed { background: #1f618d; }";
}

QString StyleHelper::getStopButtonStyle() {
    return "QPushButton { "
           "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #c0392b); "
           "   color: white; "
           "   border-radius: 4px; "
           "   font-weight: bold; "
           "   border: 1px solid #c0392b; "
           "} "
           "QPushButton:hover { background: #e74c3c; } "
           "QPushButton:pressed { background: #a93226; }";
}

QString StyleHelper::getTableStyle(bool isDark) {
    if (isDark) {
        return "QTableWidget { "
               "   background-color: #2b2b2b; "
               "   alternate-background-color: #353535; "
               "   color: white; "
               "   gridline-color: #555; "
               "   border: none; "
               "}"
               "QHeaderView::section { "
               "   background-color: #444; "
               "   color: white; "
               "   border: 1px solid #666; "
               "   padding: 4px; "
               "}"
               "QTableWidget::item:selected { background-color: #3498db; color: white; }";
    } else {
        return "QTableWidget { "
               "   background-color: white; "
               "   alternate-background-color: #f9f9f9; "
               "   color: black; "
               "   gridline-color: #ddd; "
               "   border: none; "
               "}"
               "QHeaderView::section { "
               "   background-color: #eee; "
               "   color: black; "
               "   border: 1px solid #ccc; "
               "   padding: 4px; "
               "}"
               "QTableWidget::item:selected { background-color: #3498db; color: white; }";
    }
}

QString StyleHelper::getTabStyle(bool isDark) {
    if (isDark) {
        // Темный стиль для вкладок
        return "QTabWidget::pane { border: 1px solid #444; background-color: #2b2b2b; top: -1px; } "
               "QTabBar::tab { "
               "   background: #333; "
               "   color: #aaa; "
               "   padding: 8px 15px; "
               "   border: 1px solid #444; "
               "   border-bottom: none; " // Чтобы сливалось с панелью
               "} "
               "QTabBar::tab:selected { "
               "   background: #444; "
               "   color: white; "
               "   border-bottom: 2px solid #3498db; " // Синяя полоска под активной вкладкой
               "} "
               "QTabBar::tab:hover { background: #3a3a3a; }";
    } else {
        // Светлый стиль
        return "QTabWidget::pane { border: 1px solid #ccc; top: -1px; } "
               "QTabBar::tab { "
               "   background: #e0e0e0; "
               "   color: black; "
               "   padding: 8px 15px; "
               "   border: 1px solid #ccc; "
               "} "
               "QTabBar::tab:selected { "
               "   background: white; "
               "   color: black; "
               "   border-bottom: 2px solid #3498db; "
               "}";
    }
}

QString StyleHelper::getLogStyle(bool isDark) {
    if (isDark) {
        return "QPlainTextEdit { background-color: #222; color: #0f0; border: none; font-family: Monospace; font-size: 12px; padding: 5px; }";
    } else {
        return "QPlainTextEdit { background-color: white; color: black; border: none; font-family: Monospace; font-size: 12px; padding: 5px; }";
    }
}

QString StyleHelper::getStatusBarStyle() {
    return "QWidget { background-color: #333; color: white; } "
           "QLabel { color: white; font-weight: bold; }";
}

QString StyleHelper::getProgressBarStyle() {
    return "QProgressBar { "
           "   border: none; "
           "   background: #555; "
           "   height: 8px; "
           "   border-radius: 4px; "
           "}"
           "QProgressBar::chunk { "
           "   background: #00e676; "
           "   border-radius: 4px; "
           "}";
}

