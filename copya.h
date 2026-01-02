#ifndef COPYA_H
#define COPYA_H

#include <QBrush>
#include <QFont>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "clipboard.h"
#include "database.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class Copya;
}
QT_END_NAMESPACE

class Copya : public QMainWindow
{
    Q_OBJECT

public:
    explicit Copya(QWidget *parent = nullptr);
    ~Copya();
    void init();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    //    void hideEvent(QHideEvent *event) override;

private slots:
    void refreshData();
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onEnterPressed();
    void onClearAll();
    void onDeleteSelected();
    void onMarkAsImportant();
    void onMarkAsNormal();
    void onSearchTextChanged(const QString &text);  // 搜索文本变化时的处理
    void onSearchButtonClicked();                   // 搜索按钮点击
    void onClearSearchClicked();                    // 清除搜索

private:
    void setupUI();
    void loadData();
    void showSelectedItemInfo();
    void performSearch(const QString &searchText);

    Ui::Copya *ui;
    ClipDatabase *database;
    Monitor *monitor;
    QWidget *widget_3=findChild<QWidget*>("widget_3");

private slots:
    void onPlainTextEditChanged();

private:
    bool plainTextEditVisible;
    void togglePlainTextEditVisibility();
    void saveCurrentEdit();

private:
    int currentEditingId;  // 当前正在编辑的项目ID
    bool isManualEdit;     // 标志是否是手动编辑（避免程序更新触发信号）

private slots:
    void onTreeWidgetCurrentItemChanged(QTreeWidgetItem *current,
                                        QTreeWidgetItem *previous);

private slots:
    void refreshLabelsDisplay();
    void onAddLabelButtonClicked();
    void onLabelButtonLeftClicked(int mark);
    void onLabelButtonRightClicked(int mark);

private:
    // 删除mark标识的函数
    bool deleteMark(int mark);

    // 查询函数，传入mark，传出为mark对应的文本，如果不存在则返回"普通"
    QString getLabelTextByMark(int mark);

    // 获取mark对应的所有项目
    QVector<std::tuple<int, QString, int, QString>> getDataByMark(int mark);

    // 辅助函数：将数据库返回的vector转换为QVector
    QVector<std::tuple<int, QString, int, QString>>
    convertToQtVector(const std::vector<std::tuple<int, std::string, int, std::string>>& dbVector);



};
#endif  // COPYA_H
