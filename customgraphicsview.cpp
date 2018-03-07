#include "customgraphicsview.h"
#include "mainwindow.h"
#include <QMouseEvent>
#include <QProcess>
#include <QDesktopServices>

CustomGraphicsView::CustomGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
}

CustomGraphicsView::CustomGraphicsView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{

}

void CustomGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton)
    {
        auto data = reinterpret_cast<DisplayImage*>(userData(0));
        if (data)
        {
            auto str = tr("file:") + data->fileName;
            QDesktopServices::openUrl(str);
        }
    }
}
