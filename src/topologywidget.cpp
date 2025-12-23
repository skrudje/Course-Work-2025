#include "topologywidget.h"
#include "stylehelper.h"

TopologyWidget::TopologyWidget(QWidget *parent) : QWidget(parent) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    scene = new QGraphicsScene(this);
    view = new GraphView(this);
    view->setScene(scene);
    layout->addWidget(view);

    QVBoxLayout* tools = new QVBoxLayout();
    zoomInBtn = new QPushButton("➕", this);
    zoomOutBtn = new QPushButton("➖", this);
    zoomInBtn->setFixedSize(40, 40);
    zoomOutBtn->setFixedSize(40, 40);

    connect(zoomInBtn, &QPushButton::clicked, view, &GraphView::zoomIn);
    connect(zoomOutBtn, &QPushButton::clicked, view, &GraphView::zoomOut);

    tools->addStretch();
    tools->addWidget(zoomInBtn);
    tools->addWidget(zoomOutBtn);
    tools->addStretch();
    layout->addLayout(tools);
    
    // сразу ставим центр при запуске
    setCenterNode();
}

void TopologyWidget::updateTheme(bool isDark) {
    scene->setBackgroundBrush(Qt::white);
    view->setGridColor(QColor(230, 230, 230));
    zoomInBtn->setStyleSheet(StyleHelper::getButtonStyle());
    zoomOutBtn->setStyleSheet(StyleHelper::getButtonStyle());
}

void TopologyWidget::clearMap() {
    scene->clear();
    // После очистки всегда возвращаем центральный узел
    setCenterNode();
}

void TopologyWidget::setCenterNode() {
    // 1. иконка "Меня"
    QPixmap pix("images/pc.png");
    
    // если картинки нет, нарисуем круг
    if (pix.isNull()) {
        auto ellipse = scene->addEllipse(-30, -30, 60, 60, QPen(Qt::black), QBrush(Qt::blue));
        ellipse->setZValue(10);
        ellipse->setFlag(QGraphicsItem::ItemIsMovable);
    } else {
        auto icon = scene->addPixmap(pix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        icon->setPos(-32, -32); // сдвигаем на половину размера, чтобы центр был в 0,0
        icon->setZValue(10);    // поверх линий
        icon->setFlag(QGraphicsItem::ItemIsMovable); // можно двигать мышкой
        
        // 2. подпись
        auto t = new QGraphicsTextItem("SCANNER (Я)", icon);
        t->setDefaultTextColor(Qt::black);
        QFont font = t->font();
        font.setBold(true);
        t->setFont(font);
        t->setPos(15, 65); // под иконкой
    }
}

void TopologyWidget::addDevice(QString ip, int index, QString type) {
    double radius = 300.0;
    // распределяем устройства по кругу
    double angle = index * (360.0 / 12.0) * (M_PI / 180.0);
    
    double x = radius * cos(angle);
    double y = radius * sin(angle);

    // линия от центра (0,0) к устройству
    QGraphicsLineItem* line = scene->addLine(0, 0, x, y, QPen(QColor(100, 100, 100), 2));
    line->setZValue(-1); 

    QString path = "images/unknown.png";
    if (type.contains("Windows")) path = "images/windows.png";
    else if (type.contains("Router")) path = "images/router.png";
    else if (type.contains("Mobile")) path = "images/mobile.png";
    else if (type.contains("Apple")) path = "images/mobile.png";

    QPixmap pix(path);
    if (!pix.isNull()) {
        auto icon = scene->addPixmap(pix.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        icon->setPos(x - 24, y - 24);
        icon->setFlag(QGraphicsItem::ItemIsMovable);
        icon->setZValue(5);

        auto t = new QGraphicsTextItem(ip + "\n" + type, icon);
        t->setDefaultTextColor(Qt::black);
        t->setPos(-20, 50);
    }
}