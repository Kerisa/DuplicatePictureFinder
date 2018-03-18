#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <qtwidgets/qtreewidget.h>


struct TreeWidgetFileInfo
{
    TreeWidgetFileInfo(int topIdx, QTreeWidgetItem *child, const QString & file)
        : childItem(child), topLevelIndex(topIdx), fileName(file) { }


    QString fileName;

    // 只有两层结构
    int topLevelIndex;
    QTreeWidgetItem *childItem;
};


class CustomTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit CustomTreeWidget(QWidget *parent = Q_NULLPTR);

    void                        InitTreeView();
    void                        AdjustColumeWidth();
    void                        UpdateParentItem(QTreeWidgetItem* item);
    QList<TreeWidgetFileInfo>   GetCheckedFileName();

    void                        CheckAllItem();
    void                        UncheckAllItem();

protected:
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

public slots:
    void treeItemChanged(QTreeWidgetItem* item , int column);

signals:
    void ClickItem(bool, QTreeWidgetItem*, int);
    void showMouseRightButton(const QPoint point);
};

#endif // CUSTOMTREEWIDGET_H
