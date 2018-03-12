#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QGraphicsView>

class CustomGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    CustomGraphicsView(QWidget *parent = Q_NULLPTR);
    CustomGraphicsView(QGraphicsScene *scene, QWidget *parent = Q_NULLPTR);

    virtual void mousePressEvent(QMouseEvent *event);
};

#endif // CUSTOMGRAPHICSVIEW_H
