
#include "customtreewidget.h"
#include <qevent.h>
//#include <qwidget.h>


CustomTreeWidget::CustomTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
}

void CustomTreeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    auto item = itemFromIndex(indexAt(event->pos()));
    if (item && isItemSelected(item))
    {
        emit ClickItem(event->button() & Qt::LeftButton, item, indexAt(event->pos()).column());
    }
    QTreeWidget::mouseReleaseEvent(event);
}
