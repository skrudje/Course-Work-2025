#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <cmath>
#include "graphview.h"

class TopologyWidget : public QWidget {
    Q_OBJECT

public:
    explicit TopologyWidget(QWidget *parent = nullptr);
    
    // Главные методы управления
    void clearMap();
    void setCenterNode();
    void addDevice(QString ip, int index, QString type);
    void updateTheme(bool isDark); // Обновить цвета

private:
    QGraphicsScene *scene;
    GraphView *view;
    QPushButton *zoomInBtn;
    QPushButton *zoomOutBtn;
    
    bool isDarkTheme = false;
};