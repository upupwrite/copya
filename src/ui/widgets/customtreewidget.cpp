#include "customtreewidget.h"

#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QScrollBar>

CustomTreeWidget::CustomTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    // 初始化时可以设置一些属性
    m_autoScrollOnSelection = true;     // 默认启用选择时自动滚动
    m_scrollBehavior = ScrollToCenter;  // 默认滚动到中间
    m_scrollMargin = 20;                // 滚动边距
}

void CustomTreeWidget::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    // 如果禁用自动滚动，则忽略滚动请求
    if (!m_autoScrollOnSelection && hint == EnsureVisible)
    {
        return;
    }

    // 否则调用基类的实现
    QTreeWidget::scrollTo(index, hint);
}

void CustomTreeWidget::setAutoScrollOnSelection(bool enable)
{
    m_autoScrollOnSelection = enable;
}

bool CustomTreeWidget::autoScrollOnSelection() const
{
    return m_autoScrollOnSelection;
}

// 设置焦点到第一项并自动滚动到第一项
void CustomTreeWidget::setFocusToFirstItemAndScroll()
{
    if (topLevelItemCount() == 0)
    {
        return;
    }

    QTreeWidgetItem *firstItem = topLevelItem(0);
    if (firstItem)
    {
        // 清除当前选择
        clearSelection();

        // 设置焦点到第一项
        setCurrentItem(firstItem);

        // 确保第一项可见并滚动到顶部
        scrollToItem(firstItem, QAbstractItemView::PositionAtTop);

        // 设置焦点到控件
        setFocus();
    }
}

// 检查项是否完全可见
bool CustomTreeWidget::isItemFullyVisible(QTreeWidgetItem *item) const
{
    if (!item)
        return false;

    // 获取项的视觉矩形
    QRect itemRect = visualItemRect(item);

    // 获取视口矩形
    QRect viewportRect = viewport()->rect();

    // 检查项的顶部和底部是否都在视口内
    bool topVisible = itemRect.top() >= viewportRect.top() - m_scrollMargin;
    bool bottomVisible =
        itemRect.bottom() <= viewportRect.bottom() + m_scrollMargin;

    return topVisible && bottomVisible;
}

// 检查项是否部分可见
bool CustomTreeWidget::isItemPartiallyVisible(QTreeWidgetItem *item) const
{
    if (!item)
        return false;

    QRect itemRect = visualItemRect(item);
    QRect viewportRect = viewport()->rect();

    // 检查项是否与视口相交
    return viewportRect.intersects(itemRect);
}

// 滚动到项以确保它可见
void CustomTreeWidget::scrollToEnsureVisible(QTreeWidgetItem *item)
{
    if (!item || !m_autoScrollOnSelection)
    {
        return;
    }

    // 如果项已经完全可见，不需要滚动
    if (isItemFullyVisible(item))
    {
        return;
    }

    // 根据滚动行为决定如何滚动
    switch (m_scrollBehavior)
    {
    case ScrollToTop:
        scrollToItem(item, QAbstractItemView::PositionAtTop);
        break;
    case ScrollToBottom:
        scrollToItem(item, QAbstractItemView::PositionAtBottom);
        break;
    case ScrollToCenter:
    default:
        scrollToItem(item, QAbstractItemView::PositionAtCenter);
        break;
    }
}

// 处理移动到下一项
void CustomTreeWidget::moveToNextItem()
{
    QTreeWidgetItem *current = currentItem();
    if (!current)
    {
        setFocusToFirstItemAndScroll();
        return;
    }

    // 获取下一项
    QTreeWidgetItem *nextItem = itemBelow(current);

    // 如果没有下一项，则尝试获取第一个子项（如果展开的话）
    if (!nextItem && current->isExpanded() && current->childCount() > 0)
    {
        nextItem = current->child(0);
    }

    // 如果还是没有下一项，尝试获取同级的下一个
    if (!nextItem)
    {
        QTreeWidgetItem *parent = current->parent();
        if (parent)
        {
            int index = parent->indexOfChild(current);
            if (index < parent->childCount() - 1)
            {
                nextItem = parent->child(index + 1);
            }
        }
    }

    // 如果还是没有下一项，尝试获取下一个顶级项
    if (!nextItem)
    {
        QTreeWidgetItem *parent = current->parent();
        if (parent)
        {
            // 当前项有父项，找到父项的索引
            QTreeWidgetItem *grandParent = parent->parent();
            if (grandParent)
            {
                int parentIndex = grandParent->indexOfChild(parent);
                if (parentIndex < grandParent->childCount() - 1)
                {
                    // 查找父项下一个兄弟的第一个子项
                    QTreeWidgetItem *nextParent =
                        grandParent->child(parentIndex + 1);
                    while (nextParent && nextParent->childCount() == 0)
                    {
                        parentIndex++;
                        if (parentIndex < grandParent->childCount() - 1)
                        {
                            nextParent = grandParent->child(parentIndex + 1);
                        }
                        else
                        {
                            nextParent = nullptr;
                        }
                    }
                    if (nextParent && nextParent->childCount() > 0)
                    {
                        nextItem = nextParent->child(0);
                    }
                }
            }
        }
        else
        {
            // 当前项是顶级项
            int index = indexOfTopLevelItem(current);
            if (index < topLevelItemCount() - 1)
            {
                nextItem = topLevelItem(index + 1);
            }
        }
    }

    // 如果找到下一项，选中并滚动到它
    if (nextItem)
    {
        // 清除当前选择
        clearSelection();

        // 选中下一项
        setCurrentItem(nextItem);

        // 确保下一项可见
        scrollToEnsureVisible(nextItem);

        // 如果需要，展开父项
        if (nextItem->parent() && !nextItem->parent()->isExpanded())
        {
            expandItem(nextItem->parent());
        }
    }
}

// 处理移动到前一项
void CustomTreeWidget::moveToPrevItem()
{
    QTreeWidgetItem *current = currentItem();
    if (!current)
    {
        setFocusToFirstItemAndScroll();
        return;
    }

    // 获取前一项
    QTreeWidgetItem *prevItem = itemAbove(current);

    // 如果没有前一项，尝试获取父项
    if (!prevItem)
    {
        QTreeWidgetItem *parent = current->parent();
        if (parent)
        {
            prevItem = parent;
        }
    }

    // 如果找到前一项，选中并滚动到它
    if (prevItem)
    {
        // 清除当前选择
        clearSelection();

        // 选中前一项
        setCurrentItem(prevItem);

        // 确保前一项可见
        scrollToEnsureVisible(prevItem);
    }
}

// 重写键盘事件
void CustomTreeWidget::keyPressEvent(QKeyEvent *event)
{
    // 处理特定的导航键
    switch (event->key())
    {
    case Qt::Key_Up:
        // 上箭头键 - 移动到前一项
        moveToPrevItem();
        event->accept();
        return;

    case Qt::Key_Down:
        // 下箭头键 - 移动到下一项
        moveToNextItem();
        event->accept();
        return;

    case Qt::Key_Home:
        // Home键 - 移动到第一项
        setFocusToFirstItemAndScroll();
        event->accept();
        return;

    case Qt::Key_End:
        // End键 - 移动到最后一项
        if (topLevelItemCount() > 0)
        {
            QTreeWidgetItem *lastItem = topLevelItem(topLevelItemCount() - 1);
            if (lastItem)
            {
                // 清除当前选择
                clearSelection();

                // 选中最后一项
                setCurrentItem(lastItem);

                // 确保最后一项可见并滚动到底部
                scrollToItem(lastItem, QAbstractItemView::PositionAtBottom);

                // 设置焦点
                setFocus();
            }
        }
        event->accept();
        return;

    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
        // 先让基类处理PageUp/PageDown
        QTreeWidget::keyPressEvent(event);

        // 延迟检查并确保当前项可见
        QTimer::singleShot(0, this,
                           [this]()
                           {
                               QTreeWidgetItem *current = currentItem();
                               if (current)
                               {
                                   scrollToEnsureVisible(current);
                               }
                           });
        event->accept();
        return;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        // 回车键 - 切换展开/折叠
        QTreeWidget::keyPressEvent(event);

        // 延迟确保当前项可见
        QTimer::singleShot(0, this,
                           [this]()
                           {
                               QTreeWidgetItem *current = currentItem();
                               if (current)
                               {
                                   scrollToEnsureVisible(current);
                               }
                           });
        event->accept();
        return;

    default:
        // 其他键交给基类处理
        QTreeWidget::keyPressEvent(event);

        // 检查选择变化后是否需要滚动
        QTimer::singleShot(0, this,
                           [this]()
                           {
                               QTreeWidgetItem *current = currentItem();
                               if (current && m_autoScrollOnSelection)
                               {
                                   scrollToEnsureVisible(current);
                               }
                           });
        break;
    }
}

// 重写鼠标滚轮事件
void CustomTreeWidget::wheelEvent(QWheelEvent *event)
{
    // 先让基类处理滚轮事件
    QTreeWidget::wheelEvent(event);

    // 延迟检查并确保当前项可见
    QTimer::singleShot(0, this,
                       [this]()
                       {
                           QTreeWidgetItem *current = currentItem();
                           if (current && m_autoScrollOnSelection)
                           {
                               scrollToEnsureVisible(current);
                           }
                       });
}

// 设置滚动行为
void CustomTreeWidget::setScrollBehavior(ScrollBehavior behavior)
{
    m_scrollBehavior = behavior;
}

CustomTreeWidget::ScrollBehavior CustomTreeWidget::scrollBehavior() const
{
    return m_scrollBehavior;
}

// 设置滚动边距
void CustomTreeWidget::setScrollMargin(int margin)
{
    m_scrollMargin = qMax(0, margin);
}

int CustomTreeWidget::scrollMargin() const { return m_scrollMargin; }

// 聚焦并滚动到特定项
void CustomTreeWidget::focusAndScrollToItem(QTreeWidgetItem *item)
{
    if (!item)
        return;

    // 清除当前选择
    clearSelection();

    // 选中指定项
    setCurrentItem(item);

    // 确保项可见
    scrollToEnsureVisible(item);

    // 设置焦点
    setFocus();
}

// 获取当前视口中完全可见的项的数量
int CustomTreeWidget::visibleItemCount() const
{
    QRect viewportRect = viewport()->rect();
    int count = 0;

    // 遍历所有项，检查是否在视口中
    for (int i = 0; i < topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = topLevelItem(i);
        if (item && isItemFullyVisible(item))
        {
            count++;
        }

        // 递归检查子项
        count += countVisibleChildren(item, viewportRect);
    }

    return count;
}

// 递归计算可见子项数量
int CustomTreeWidget::countVisibleChildren(QTreeWidgetItem *parent,
                                           const QRect &viewportRect) const
{
    if (!parent || !parent->isExpanded())
    {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < parent->childCount(); ++i)
    {
        QTreeWidgetItem *child = parent->child(i);
        if (child && isItemFullyVisible(child))
        {
            count++;
        }

        // 递归检查子项的子项
        count += countVisibleChildren(child, viewportRect);
    }

    return count;
}
