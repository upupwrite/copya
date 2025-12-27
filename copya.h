#ifndef COPYA_H
#define COPYA_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QBrush>
#include <QFont>
#include "database.h"
#include "clipboard.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Copya; }
QT_END_NAMESPACE

class Copya : public QMainWindow
{
    Q_OBJECT

public:
    explicit Copya(QWidget *parent = nullptr);
    ~Copya();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void refreshData();
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onEnterPressed();
    void onClearAll();
    void onDeleteSelected();
    void onMarkAsImportant();
    void onMarkAsNormal();

private:
    void setupUI();
    void loadData();
    void showSelectedItemInfo();

    Ui::Copya *ui;
    ClipDatabase *database;
    Monitor *monitor;

};
#endif // COPYA_H
