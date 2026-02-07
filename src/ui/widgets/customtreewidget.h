#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <QTimer>
#include <QTreeWidget>

class CustomTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    enum ScrollBehavior
    {
        ScrollToTop,
        ScrollToCenter,
        ScrollToBottom
    };

    explicit CustomTreeWidget(QWidget *parent = nullptr);
    // 鼠标滚轮自动滚动控制
    void setWheelAutoScroll(bool enable);
    bool wheelAutoScroll() const;
    // 重写滚动方法
    void scrollTo(const QModelIndex &index,
                  ScrollHint hint = EnsureVisible) override;

    // 自动滚动设置
    void setAutoScrollOnSelection(bool enable);
    bool autoScrollOnSelection() const;

    // 设置焦点到第一项并自动滚动
    void setFocusToFirstItemAndScroll();

    // 移动到下一项
    void moveToNextItem();

    // 移动到前一项
    void moveToPrevItem();

    // 聚焦并滚动到特定项
    void focusAndScrollToItem(QTreeWidgetItem *item);

    // 设置滚动行为
    void setScrollBehavior(ScrollBehavior behavior);
    ScrollBehavior scrollBehavior() const;

    // 设置滚动边距
    void setScrollMargin(int margin);
    int scrollMargin() const;

    // 检查项是否完全可见
    bool isItemFullyVisible(QTreeWidgetItem *item) const;

    // 检查项是否部分可见
    bool isItemPartiallyVisible(QTreeWidgetItem *item) const;

    // 获取可见项数量
    int visibleItemCount() const;

protected:
    void wheelEvent(QWheelEvent *event) override;
    // 重写事件处理
    void keyPressEvent(QKeyEvent *event) override;

private:
    // 滚动到项以确保可见
    void scrollToEnsureVisible(QTreeWidgetItem *item);

    // 递归计算可见子项数量
    int countVisibleChildren(QTreeWidgetItem *parent,
                             const QRect &viewportRect) const;
    bool m_enableWheelAutoScroll;  // 控制鼠标滚轮是否触发自动滚动
    bool m_autoScrollOnSelection;
    ScrollBehavior m_scrollBehavior;
    int m_scrollMargin;
};

#endif  // CUSTOMTREEWIDGET_H
