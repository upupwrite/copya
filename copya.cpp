#include "copya.h"
#include "./ui_copya.h"
#include <QKeyEvent>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>


Copya::Copya(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::Copya),
    database(nullptr)
{
    // 初始化数据库
    database = new ClipDatabase();
    monitor = new Monitor();

    monitor->start();

    if (!database->isOpen()) {
        QMessageBox::critical(this, "错误", "无法打开数据库！");
        return;
    }

    ui->setupUi(this);

    ui->treeWidget->setColumnCount(2);
    QStringList headers;
    headers << "内容" << "标记";
    ui->treeWidget->setHeaderLabels(headers);
    // 设置列宽
       ui->treeWidget->setColumnWidth(0, 600);
       ui->treeWidget->setColumnWidth(1, 100);

       // 启用交替行颜色
       ui->treeWidget->setAlternatingRowColors(true);

       // 设置选择模式
       ui->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);

       // 设置双击事件
       connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked,
               this, &Copya::onItemDoubleClicked);

       // 设置键盘事件过滤器
       ui->treeWidget->installEventFilter(this);

    connect(ui->refreshButton,&QToolButton::clicked,this,&Copya::refreshData);

    connect(ui->deleteButton,&QToolButton::clicked,this,&Copya::onDeleteSelected);

    connect(ui->markImportantButton,&QToolButton::clicked,this,&Copya::onMarkAsImportant);

    connect(ui->markNormalButton,&QToolButton::clicked,this,&Copya::onMarkAsNormal);

    connect(monitor, &Monitor::clipTextChanged, this, &Copya::refreshData);
    // 安装事件过滤器
    ui->treeWidget->installEventFilter(this);

    // 连接双击信号
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, &Copya::onItemDoubleClicked);

    loadData();
}

Copya::~Copya()
{
    delete ui;
    if (database) {
        delete database;
    }
}

void Copya::refreshData()
{
    loadData();
}

void Copya::onDeleteSelected()
{
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "提示", "请先选择一个项目！");
        return;
    }

    int id = currentItem->data(0, Qt::UserRole + 1).toInt();

    int ret = QMessageBox::question(this, "确认",
        "确定要删除选中的项目吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (database->deleteById(id)) {
            delete currentItem;
            QMessageBox::information(this, "成功", "项目已删除！");
        } else {
            QMessageBox::warning(this, "错误", "删除项目失败！");
        }
    }
}

void Copya::onMarkAsImportant()
{
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "提示", "请先选择一个项目！");
        return;
    }

    int id = currentItem->data(0, Qt::UserRole + 1).toInt();

    if (database->setMarkById(id, 1)) {
        currentItem->setText(1, "重要");
        currentItem->setForeground(1, QBrush(Qt::red));

        QFont font = currentItem->font(0);
        font.setBold(true);
        currentItem->setFont(0, font);
    } else {
        QMessageBox::warning(this, "错误", "标记为重要失败！");
    }
}

void Copya::onMarkAsNormal()
{
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "提示", "请先选择一个项目！");
        return;
    }

    int id = currentItem->data(0, Qt::UserRole + 1).toInt();

    if (database->setMarkById(id, 0)) {
        currentItem->setText(1, "普通");
        currentItem->setForeground(1, QBrush());

        QFont font = currentItem->font(0);
        font.setBold(false);
        currentItem->setFont(0, font);
    } else {
        QMessageBox::warning(this, "错误", "标记为普通失败！");
    }
}

void Copya::loadData()
{
    ui->treeWidget->clear();

    auto data = database->readAllData();
    if (data.empty()) {
        return;
    }

    // 使用反向迭代器实现倒序显示
    for (auto it = data.rbegin(); it != data.rend(); ++it) {
        const auto& item = *it;
        int id = std::get<0>(item);
        std::string content = std::get<1>(item);
        int mark = std::get<2>(item);

        QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget);

        // 设置内容，如果内容太长则截断
        QString contentText = QString::fromStdString(content);
        if (contentText.length() > 200) {
            contentText = contentText.left(200) + "...";
        }
        treeItem->setText(0, contentText);

        // 设置标记
        QString markText;
        if (mark == 1) {
            markText = "重要";
            treeItem->setForeground(1, QBrush(Qt::red));
        } else if (mark == 0) {
            markText = "普通";
        } else {
            markText = QString::number(mark);
        }
        treeItem->setText(1, markText);

        // 存储完整的原始内容
        treeItem->setData(0, Qt::UserRole, QString::fromStdString(content));
        treeItem->setData(0, Qt::UserRole + 1, id);

        // 根据标记设置字体
        if (mark == 1) {
            QFont font = treeItem->font(0);
            font.setBold(true);
            treeItem->setFont(0, font);
        }
    }

    // 自动调整列宽
    ui->treeWidget->header()->setStretchLastSection(false);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
}

bool Copya::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->treeWidget && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            onEnterPressed();
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void Copya::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    Q_UNUSED(item);
    showSelectedItemInfo();
}

void Copya::onEnterPressed()
{
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (currentItem) {
        QString fullContent = currentItem->data(0, Qt::UserRole).toString();
        // 将内容复制到剪贴板
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(fullContent);
        qDebug() << "Content copied to clipboard:" << fullContent;

        // 显示复制成功的提示
        qDebug()<<"设置剪切板文本成功";
    } else {
        QMessageBox::information(this, "提示", "请先选择一个项目！");
    }
}

void Copya::showSelectedItemInfo()
{
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "提示", "请先选择一个项目！");
        return;
    }

    QString content = currentItem->data(0, Qt::UserRole).toString();
    int id = currentItem->data(0, Qt::UserRole + 1).toInt();

    // 打印到控制台
    qDebug() << "ID:" << id;
    qDebug() << "内容:" << content;

    // 在界面上显示信息框
    ui->plainTextEdit->setPlainText(content);
}

void Copya::onClearAll()
{
    int ret = QMessageBox::question(this, "确认",
        "确定要清空所有数据吗？此操作不可撤销！",
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (database->clearAllData()) {
            loadData();
            QMessageBox::information(this, "成功", "数据已清空！");
        } else {
            QMessageBox::warning(this, "错误", "清空数据失败！");
        }
    }
}

