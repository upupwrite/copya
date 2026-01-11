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

#include "core/clipboard.h"
#include "core/database.h"

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
    void onSearchTextChanged(const QString &text);
    void onSearchButtonClicked();
    void onClearSearchClicked();
    void onMarkAsIt(int mark);
    void onPlainTextEditChanged();
    void onTreeWidgetCurrentItemChanged(QTreeWidgetItem *current,
                                        QTreeWidgetItem *previous);
    void refreshLabelsDisplay();
    void onAddLabelButtonClicked();
    void onLabelButtonRightClicked(int mark);

private:
    Ui::Copya *ui;
    ClipDatabase *database;
    Monitor *monitor;
    QWidget *widget_3 = findChild<QWidget *>("widget_3");
    std::vector<std::pair<int, std::string>> marks;
    void setupUI();
    void loadData();
    void loadMarkData(int mark);
    void showSelectedItemInfo();
    void performSearch(const QString &searchText);
    bool plainTextEditVisible;
    void togglePlainTextEditVisibility();
    void saveCurrentEdit();
    int currentEditingId;
    bool isManualEdit;
    int currentShowMark = -1;
    bool deleteMark(int mark);
    QString getLabelTextByMark(int mark);
    QVector<std::tuple<int, QString, int, QString>> getDataByMark(int mark);
    QVector<std::tuple<int, QString, int, QString>> convertToQtVector(
        const std::vector<std::tuple<int, std::string, int, std::string>>
            &dbVector);

    void refreshMarkData();
};
#endif  // COPYA_H
