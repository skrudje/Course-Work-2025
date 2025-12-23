#pragma once
#include <QString>

class StyleHelper {
public:
    static QString getMainWindowStyle(); 
    static QString getLabelStyle();
    static QString getLineEditStyle(bool isDark);
    static QString getButtonStyle();
    static QString getStartButtonStyle();
    static QString getStopButtonStyle();
    static QString getTableStyle(bool isDark);
    static QString getTabStyle(bool isDark); 
    static QString getLogStyle(bool isDark);
    static QString getStatusBarStyle();
    static QString getProgressBarStyle();
};