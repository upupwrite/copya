#include "customtreewidget.h"

CustomTreeWidget::CustomTreeWidget(QWidget *parent) : QTreeWidget(parent) {}

void CustomTreeWidget::scrollToItem(const QTreeWidgetItem *item,
                                    QAbstractItemView::ScrollHint hint)
{
    // 不执行默认的滚动逻辑
    Q_UNUSED(item);
    Q_UNUSED(hint);
}
