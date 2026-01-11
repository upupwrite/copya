#include "customtreewidget.h"

CustomTreeWidget::CustomTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    // 初始化时可以设置一些属性
    m_autoScrollOnSelection = false;
}

void CustomTreeWidget::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    // 如果禁用自动滚动，则忽略滚动请求
    if (!m_autoScrollOnSelection && hint == EnsureVisible)
    {
        // 可以什么都不做，或者只处理特定类型的滚动
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
