
#include "customtreewidget.h"
#include <qevent.h>


CustomTreeWidget::CustomTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    auto b = connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(treeItemChanged(QTreeWidgetItem*,int)));
    Q_ASSERT(b);
}

void CustomTreeWidget::InitTreeView()
{
    setColumnCount(4);

    QStringList list;
    list.push_back("Filename");
    list.push_back("Size");
    list.push_back("Solution");
    list.push_back("ModifyTime");
    setHeaderLabels(list);

    AdjustColumeWidth();
}

void CustomTreeWidget::AdjustColumeWidth()
{
    //设置列宽
    constexpr int colWidth[4] = {700, 120, 100, 160};
    constexpr int otherColWidth = colWidth[1] + colWidth[2] + colWidth[3];

    setColumnWidth(0, width() * 0.6);
    setColumnWidth(1, width() * 0.1);
    setColumnWidth(2, width() * 0.1);
    setColumnWidth(3, width() * 0.16);
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


void CustomTreeWidget::treeItemChanged(QTreeWidgetItem* item, int column)
{
    if (Qt::Checked == item->checkState(0))
    {
        int count = item->childCount(); //返回子项的个数
        if (count > 0)
        {
            for (int i = 0; i < count; ++i)
            {
                item->child(i)->setCheckState(0, Qt::Checked);
            }
        }
        else
        {
            updateParentItem(item);
        }
    }
    else if (Qt::Unchecked == item->checkState(0))
    {
        int count = item->childCount();
        if (count > 0)
        {
            for (int i = 0; i < count; ++i)
            {
                item->child(i)->setCheckState(0,Qt::Unchecked);
            }
        }
        else
        {
            updateParentItem(item);
        }
    }
}

void CustomTreeWidget::updateParentItem(QTreeWidgetItem* item)
{
    QTreeWidgetItem *parent = item->parent();
    if (parent == NULL)
    {
        return;
    }
    int selectedCount = 0;
    int childCount = parent->childCount();
    for (int i=0; i<childCount; i++) //判断有多少个子项被选中
    {
        QTreeWidgetItem* childItem = parent->child(i);
        if (childItem->checkState(0) == Qt::Checked)
        {
            selectedCount++;
        }
    }
    if (selectedCount <= 0)  //如果没有子项被选中，父项设置为未选中状态
    {
        parent->setCheckState(0,Qt::Unchecked);
    }
    else if (selectedCount>0 && selectedCount<childCount)    //如果有部分子项被选中，父项设置为部分选中状态，即用灰色显示
    {
        parent->setCheckState(0,Qt::PartiallyChecked);
    }
    else if (selectedCount == childCount)    //如果子项全部被选中，父项则设置为选中状态
    {
        parent->setCheckState(0,Qt::Checked);
    }
}

QList<TreeWidgetFileInfo> CustomTreeWidget::GetCheckedFileName()
{
    QList<TreeWidgetFileInfo> fileList;

    for (int t = 0; t < topLevelItemCount(); ++t)
    {
        auto topItem = topLevelItem(t);
        for (int i = 0; i < topItem->childCount(); ++i)
        {
            QTreeWidgetItem* childItem = topItem->child(i);
            if (childItem->checkState(0) == Qt::Checked)
            {
                fileList.push_back(TreeWidgetFileInfo(t, childItem, childItem->text(0)));
            }
        }
    }

    return fileList;
}


//void showMouseRightButton(const QPoint &point)
//{
//    QMenu *qMenu = new QMenu(treeView);
//    QModelIndex indexSelect = treeView->indexAt(point);//获得当前右击事件的节点

//    QModelIndex indexParent = indexSelect.parent();//获得当前右击事件节点的父节点
//    int row = indexSelect.row();//获得当前右击事件的树形节点所在的行
//    QModelIndex indexData = indexParent.child(row,1);//获得当前右击事件节点父节点的第row行的第二列的节点
//    QString QStr;
//    QStr = indexSelect.data().toString();//获得当前右击事件的节点数据
//    if(QStr != "")//如果当前右击事件在节点范围内，即树形节点上则显示右键的菜单
//    {
//        QAction* showAction = new QAction("&菜单显示",this);
//        qMenu->addAction(showAction);

//        connect(showAction, SIGNAL(triggered()), this, SLOT(showCurve()));//showCurve为点击菜单右键“菜单显示”的槽函数
//        qMenu->exec(QCursor::pos());
//    }
//}
