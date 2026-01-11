#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <QStyleOptionViewItem>
#include <QTreeWidget>

class CustomTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit CustomTreeWidget(QWidget *parent = nullptr);

    // 重写scrollTo方法，禁用自动滚动
    void scrollTo(const QModelIndex &index,
                  ScrollHint hint = EnsureVisible) override;

    // 可选：禁用自动滚动到选择项
    void setAutoScrollOnSelection(bool enable);
    bool autoScrollOnSelection() const;

protected:
    bool m_autoScrollOnSelection = false;
};

#endif  // CUSTOMTREEWIDGET_H
