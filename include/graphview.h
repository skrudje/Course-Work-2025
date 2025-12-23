#pragma once
#include <QGraphicsView>
#include <QWheelEvent>
#include <QPainter> 

class GraphView : public QGraphicsView {
public:
    GraphView(QWidget* parent = nullptr) : QGraphicsView(parent) {
        setDragMode(QGraphicsView::ScrollHandDrag); 
        setRenderHint(QPainter::Antialiasing);      
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setStyleSheet("border: 0px;");
    }

    void zoomIn() { scale(1.15, 1.15); }
    void zoomOut() { scale(1.0 / 1.15, 1.0 / 1.15); }
    
    void setGridColor(QColor color) {
        gridColor = color;
        viewport()->update(); 
    }

protected:
    QColor gridColor = QColor(230, 230, 230); 

    void wheelEvent(QWheelEvent* event) override {
        event->ignore(); 
    }

    void drawBackground(QPainter *painter, const QRectF &rect) override {
        QGraphicsView::drawBackground(painter, rect);
        
        painter->setPen(QPen(gridColor, 1));
        int gridSize = 50; 
        
        qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
        qreal top = int(rect.top()) - (int(rect.top()) % gridSize);

        QVector<QLineF> lines;
        for (qreal x = left; x < rect.right(); x += gridSize)
            lines.append(QLineF(x, rect.top(), x, rect.bottom()));
        for (qreal y = top; y < rect.bottom(); y += gridSize)
            lines.append(QLineF(rect.left(), y, rect.right(), y));

        painter->drawLines(lines);
    }
};