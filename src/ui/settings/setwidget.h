#pragma once

#include <QPointer>
#include <QWidget>
#include <memory>

/**
 * @brief 设置widget到父组件中，自动清理原有内容
 *
 * @tparam T 父组件类型（必须派生自QWidget）
 * @tparam U 子组件类型（必须派生自QWidget）
 * @param parent 父组件指针
 * @param child 要设置的子组件
 * @param keepSameWidget 如果传入相同的widget是否保持不重新设置
 * @return U* 返回设置的子组件指针
 */
template <typename T, typename U>
U* setChildWidget(T* parent, U* child, bool keepSameWidget = true)
{
    static_assert(std::is_base_of<QWidget, T>::value,
                  "Parent type must inherit from QWidget");
    static_assert(std::is_base_of<QWidget, U>::value,
                  "Child type must inherit from QWidget");

    if (!parent)
    {
        qWarning() << "Parent widget is null";
        return nullptr;
    }

    // 获取当前已存在的子widget
    QLayout* layout = parent->layout();
    QWidget* currentChild = nullptr;

    if (layout && layout->count() > 0)
    {
        QLayoutItem* item = layout->itemAt(0);
        if (item)
        {
            currentChild = item->widget();
        }
    }

    // 检查是否和当前widget相同
    if (keepSameWidget && currentChild == child)
    {
        return static_cast<U*>(currentChild);
    }

    // 清理原有内容
    if (layout)
    {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr)
        {
            if (item->widget())
            {
                item->widget()->deleteLater();
            }
            delete item;
        }
    }
    else
    {
        // 如果没有布局，直接删除所有子widget
        QList<QWidget*> children = parent->findChildren<QWidget*>(
            QString(), Qt::FindDirectChildrenOnly);
        for (QWidget* w : children)
        {
            w->deleteLater();
        }
    }

    // 设置新的widget
    if (child)
    {
        if (!layout)
        {
            // 创建新的布局
            QVBoxLayout* newLayout = new QVBoxLayout(parent);
            newLayout->setContentsMargins(0, 0, 0, 0);
            parent->setLayout(newLayout);
            layout = newLayout;
        }

        layout->addWidget(child);
        child->show();
    }

    return child;
}

/**
 * @brief 智能指针版本的设置widget函数
 */
template <typename T, typename U>
U* setChildWidget(T* parent, std::unique_ptr<U>& child,
                  bool keepSameWidget = true)
{
    return setChildWidget(parent, child.get(), keepSameWidget);
}

/**
 * @brief QPointer版本的设置widget函数
 */
template <typename T, typename U>
U* setChildWidget(T* parent, QPointer<U>& child, bool keepSameWidget = true)
{
    return setChildWidget(parent, child.data(), keepSameWidget);
}

/**
 * @brief 设置widget到父组件中的指定布局位置
 *
 * @param parent 父组件
 * @param child 子组件
 * @param index 在布局中的位置
 * @param keepSameWidget 如果传入相同的widget是否保持不重新设置
 * @return QWidget* 返回设置的子组件
 */
template <typename T>
QWidget* setChildWidgetAt(T* parent, QWidget* child, int index = 0,
                          bool keepSameWidget = true)
{
    if (!parent)
        return nullptr;

    QLayout* layout = parent->layout();
    if (!layout)
    {
        // 创建默认布局
        QVBoxLayout* newLayout = new QVBoxLayout(parent);
        newLayout->setContentsMargins(0, 0, 0, 0);
        parent->setLayout(newLayout);
        layout = newLayout;
    }

    // 检查指定位置的widget是否相同
    if (keepSameWidget)
    {
        QLayoutItem* item = layout->itemAt(index);
        if (item && item->widget() == child)
        {
            return child;
        }
    }

    // 删除指定位置的widget
    QLayoutItem* oldItem = layout->takeAt(index);
    if (oldItem)
    {
        if (oldItem->widget())
        {
            oldItem->widget()->deleteLater();
        }
        delete oldItem;
    }

    // 插入新的widget
    if (child)
    {
        layout->insertWidget(index, child);
        child->show();
    }

    return child;
}
