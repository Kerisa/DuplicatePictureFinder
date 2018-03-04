#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <qtwidgets/qtreewidget.h>


class CustomTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit CustomTreeWidget(QWidget *parent = Q_NULLPTR);
    void InitTreeView();
    void updateParentItem(QTreeWidgetItem* item);

protected:
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

public slots:
    void treeItemChanged(QTreeWidgetItem* item , int column);

signals:
    void ClickItem(bool, QTreeWidgetItem*, int);
    void showMouseRightButton(const QPoint point);
};

#endif // CUSTOMTREEWIDGET_H
