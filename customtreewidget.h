#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <QObject>
#include <QTreeWidget>

class CustomTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit CustomTreeWidget(QWidget *parent = nullptr);

protected:
    void scrollToItem(const QTreeWidgetItem *item,
                      QAbstractItemView::ScrollHint hint =
                          QAbstractItemView::EnsureVisible) override;
};

#endif  // CUSTOMTREEWIDGET_H
