#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <qtwidgets/qtreewidget.h>


class CustomTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit CustomTreeWidget(QWidget *parent = Q_NULLPTR);

protected:
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

signals:
    void ClickItem(bool, QTreeWidgetItem*, int);
};

#endif // CUSTOMTREEWIDGET_H
