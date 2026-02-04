#include "copya.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QSize>
#include <QSpacerItem>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "./ui_copya.h"
#include "ui/qss/default.h"

Copya::Copya(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::Copya),
      database(nullptr),
      plainTextEditVisible(true),
      currentEditingId(-1),
      isManualEdit(false)
{
    ui->setupUi(this);

    // 初始化数据库
    database = new ClipDatabase();
    monitor = new Monitor();

    monitor->start();

    if (!database->isOpen())
    {
        QMessageBox::critical(this, "错误", "无法打开数据库！");
        return;
    }
    // 从数据库获取所有marks和标签
    marks = database->getAllMarksAndLabels();

    // 设置按ESC键隐藏窗口
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 程序启动时隐藏窗口（默认隐藏）
    this->hide();

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
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this,
            &Copya::onItemDoubleClicked);

    // 设置键盘事件过滤器
    ui->treeWidget->installEventFilter(this);

    connect(ui->refreshButton, &QToolButton::clicked, this,
            &Copya::refreshData);

    connect(ui->deleteButton, &QToolButton::clicked, this,
            &Copya::onDeleteSelected);

    connect(ui->markImportantButton, &QToolButton::clicked, this,
            &Copya::onMarkAsImportant);

    connect(ui->markNormalButton, &QToolButton::clicked, this,
            &Copya::onMarkAsNormal);

    connect(monitor, &Monitor::clipTextChanged, this, &Copya::refreshData);
    // 安装事件过滤器
    ui->treeWidget->installEventFilter(this);

    // 连接双击信号
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this,
            &Copya::onItemDoubleClicked);
    connect(ui->lineEdit, &QLineEdit::textChanged, this,
            &Copya::onSearchTextChanged);
    connect(ui->clearButton, &QToolButton::clicked, this,
            &Copya::onClearSearchClicked);
    // 添加 plainTextEdit 内容改变的信号连接
    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this,
            &Copya::onPlainTextEditChanged);

    // 初始化时隐藏 plainTextEdit（可选）
    ui->plainTextEdit->setVisible(true);

    // 连接树形控件当前项目改变的信号
    connect(ui->treeWidget, &QTreeWidget::currentItemChanged, this,
            &Copya::onTreeWidgetCurrentItemChanged);

    connect(ui->addLabelButton, &QToolButton::clicked, this,
            &Copya::onAddLabelButtonClicked);
    ui->deleteButton->setAutoRaise(true);
    ui->refreshButton->setAutoRaise(true);
    ui->markImportantButton->setAutoRaise(true);
    ui->markNormalButton->setAutoRaise(true);

    ui->deleteButton->setAutoRaise(true);
    ui->refreshButton->setAutoRaise(true);
    ui->markImportantButton->setAutoRaise(true);
    ui->markNormalButton->setAutoRaise(true);
    ui->clearButton->setAutoRaise(true);

    ui->markImportantButton->setStyleSheet(
        ui->markImportantButton->styleSheet() + DefaultQss::TOOLSET);
    ui->markNormalButton->setStyleSheet(ui->markImportantButton->styleSheet() +
                                        DefaultQss::TOOLSET);
    ui->deleteButton->setStyleSheet(ui->markImportantButton->styleSheet() +
                                    DefaultQss::TOOLSET);
    ui->refreshButton->setStyleSheet(ui->markImportantButton->styleSheet() +
                                     DefaultQss::TOOLSET);
    ui->clearButton->setStyleSheet(ui->markImportantButton->styleSheet() +
                                   DefaultQss::TOOLSET);

    QMenu *contextImportantMenu = new QMenu(ui->markImportantButton);
    QAction *viewImportantAction = contextImportantMenu->addAction("查看项目");
    connect(viewImportantAction, &QAction::triggered, this,
            [this]() { onLabelButtonRightClicked(Marks::IMPORTANTMARK); });

    QMenu *contextNormalMenu = new QMenu(ui->markNormalButton);
    QAction *viewNormalAction = contextNormalMenu->addAction("查看项目");
    connect(viewNormalAction, &QAction::triggered, this,
            [this]() { onLabelButtonRightClicked(Marks::NORMALMARK); });

    ui->markImportantButton->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->markNormalButton->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->markImportantButton, &QPushButton::customContextMenuRequested,
            this,
            [this, contextImportantMenu](const QPoint &pos)
            {
                contextImportantMenu->exec(
                    ui->markImportantButton->mapToGlobal(pos));
            });

    connect(
        ui->markNormalButton, &QPushButton::customContextMenuRequested, this,
        [this, contextNormalMenu](const QPoint &pos)
        { contextNormalMenu->exec(ui->markNormalButton->mapToGlobal(pos)); });

    refreshLabelsDisplay();
    loadData();
}

Copya::~Copya()
{
    delete ui;
    if (database)
    {
        delete database;
    }
}

void Copya::refreshMarkData() { marks = database->getAllMarksAndLabels(); }

void Copya::onMarkAsIt(int mark)
{
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    QString labelText = getLabelTextByMark(mark);
    qDebug() << "左键单击标签 - Mark:" << mark << "标签:" << labelText;
    if (!currentItem)
    {
        QMessageBox::information(this, "提示", "请先选择一个项目！");
        return;
    }

    int id = currentItem->data(0, Qt::UserRole + 1).toInt();

    if (database->setMarkById(id, mark))
    {
        currentItem->setText(1, labelText);
    }
    else
    {
        QMessageBox::warning(this, "错误", "标记失败！");
    }
}

void Copya::onMarkAsImportant()
{
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (!currentItem)
    {
        QMessageBox::information(this, "提示", "请先选择一个项目！");
        return;
    }

    int id = currentItem->data(0, Qt::UserRole + 1).toInt();

    if (database->setMarkById(id, 1))
    {
        currentItem->setText(1, Marks::IMPORTANTMARKTEXT);
        currentItem->setForeground(1, QBrush(Qt::red));

        QFont font = currentItem->font(0);
        font.setBold(true);
        currentItem->setFont(0, font);
    }
    else
    {
        QMessageBox::warning(this, "错误", "标记为重要失败！");
    }
}

void Copya::onMarkAsNormal()
{
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (!currentItem)
    {
        QMessageBox::information(this, "提示", "请先选择一个项目！");
        return;
    }

    int id = currentItem->data(0, Qt::UserRole + 1).toInt();

    if (database->setMarkById(id, 0))
    {
        currentItem->setText(1, Marks::NORMALMARKTEXT);
        currentItem->setForeground(1, QBrush());

        QFont font = currentItem->font(0);
        font.setBold(false);
        currentItem->setFont(0, font);
    }
    else
    {
        QMessageBox::warning(this, "错误", "标记为普通失败！");
    }
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
    if (currentItem)
    {
        QString fullContent = currentItem->data(0, Qt::UserRole).toString();
        // 将内容复制到剪贴板
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(fullContent);
        qDebug() << "Content copied to clipboard:" << fullContent;

        // 显示复制成功的提示
        qDebug() << "设置剪切板文本成功";

        // 不关闭程序，只是隐藏窗口
        this->hide();
    }
    else
    {
        QMessageBox::information(this, "提示", "请先选择一个项目！");
    }
}

void Copya::showSelectedItemInfo()
{
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (!currentItem)
    {
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

    if (ret == QMessageBox::Yes)
    {
        if (database->clearAllData())
        {
            loadData();
            QMessageBox::information(this, "成功", "数据已清空！");
        }
        else
        {
            QMessageBox::warning(this, "错误", "清空数据失败！");
        }
    }
}

// 搜索文本变化时的处理
void Copya::onSearchTextChanged(const QString &text)
{
    if (text.isEmpty())
    {
        // 如果搜索框为空，刷新显示所有数据
        refreshData();
        return;
    }

    // 可选：延时搜索，避免每次输入都立即搜索
    // 这里直接搜索
    performSearch(text);
}

// 清除搜索
void Copya::onClearSearchClicked(bool isRefreshData)
{
    if (ui->lineEdit->text().isEmpty())
    {
        return;
    }
    ui->lineEdit->clear();
    if (isRefreshData)
    {
        refreshData();
    }
}

// 执行搜索的核心函数
void Copya::performSearch(const QString &searchText)
{
    // 保存当前编辑的内容
    saveCurrentEdit();

    if (!database->isOpen())
    {
        QMessageBox::warning(this, "错误", "数据库未打开！");
        return;
    }

    if (searchText.isEmpty())
    {
        loadData();
        return;
    }

    // 清空当前显示
    ui->treeWidget->clear();

    // 调用数据库的搜索函数
    std::string searchTerm = searchText.toStdString();
    auto searchResults = database->searchText(searchTerm, false,
                                              false);  // 不区分大小写，模糊匹配

    if (searchResults.empty())
    {
        // 如果没找到结果，显示提示
        QTreeWidgetItem *noResultItem = new QTreeWidgetItem(ui->treeWidget);
        noResultItem->setText(0, "未找到匹配项");
        noResultItem->setText(1, "");
        noResultItem->setData(0, Qt::UserRole, QString(""));
        noResultItem->setData(0, Qt::UserRole + 1, -1);
        noResultItem->setFlags(noResultItem->flags() & ~Qt::ItemIsSelectable);
        return;
    }

    // 显示搜索结果（倒序显示）
    for (auto it = searchResults.rbegin(); it != searchResults.rend(); ++it)
    {
        const auto &item = *it;
        int id = std::get<0>(item);
        std::string content = std::get<1>(item);
        int mark = std::get<2>(item);
        std::string createdTime = std::get<3>(item);

        QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget);

        // 设置内容（高亮搜索词）
        QString contentText = QString::fromStdString(content);
        QString searchTextLower = searchText.toLower();
        QString contentLower = contentText.toLower();

        // 如果内容太长则截断
        if (contentText.length() > 200)
        {
            contentText = contentText.left(200) + "...";
        }

        // 设置高亮显示
        if (searchText.length() >= 2)
        {  // 只有搜索词长度>=2时才高亮
            int pos = contentLower.indexOf(searchTextLower);
            if (pos >= 0)
            {
                QString highlighted = contentText;
                highlighted.insert(pos + searchText.length(), "</span>");
                highlighted.insert(
                    pos,
                    "<span style=\"background-color: yellow; color: black;\">");
                treeItem->setText(0, highlighted);
                treeItem->setToolTip(0, "<html>" + highlighted + "</html>");
            }
            else
            {
                treeItem->setText(0, contentText);
                treeItem->setToolTip(0, contentText);
            }
        }
        else
        {
            treeItem->setText(0, contentText);
            treeItem->setToolTip(0, contentText);
        }

        // 设置标记
        QString markText;
        if (mark == 1)
        {
            markText = Marks::IMPORTANTMARKTEXT;
            treeItem->setForeground(1, QBrush(Qt::red));
        }
        else if (mark == 0)
        {
            markText = Marks::NORMALMARKTEXT;
        }
        else
        {
            markText = QString::number(mark);
        }
        treeItem->setText(1, markText);

        // 存储完整的原始内容
        treeItem->setData(0, Qt::UserRole, QString::fromStdString(content));
        treeItem->setData(0, Qt::UserRole + 1, id);

        // 根据标记设置字体
        if (mark == 1)
        {
            QFont font = treeItem->font(0);
            font.setBold(true);
            treeItem->setFont(0, font);
        }
    }

    // 自动调整列宽
    ui->treeWidget->header()->setStretchLastSection(false);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);

    // 更新状态显示
    ui->statusbar->showMessage(
        QString("找到 %1 条匹配结果").arg(searchResults.size()), 3000);

    // 重置当前编辑ID
    currentEditingId = -1;
    ui->plainTextEdit->clear();
}

bool Copya::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->treeWidget && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Enter)
        {
            onEnterPressed();
            return true;
        }
    }

    // 添加 ESC 键隐藏窗口的处理（全局）
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            saveCurrentEdit();  // 保存当前编辑的内容
            this->hide();
            return true;
        }
        // 添加 F4 键处理
        if (keyEvent->key() == Qt::Key_F4)
        {
            togglePlainTextEditVisibility();
            return true;
        }
        if (keyEvent->key() == Qt::Key_F12)
        {
            currentShowMark = -1;
            onClearSearchClicked(false);
            refreshData();
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void Copya::togglePlainTextEditVisibility()
{
    plainTextEditVisible = !plainTextEditVisible;
    ui->plainTextEdit->setVisible(plainTextEditVisible);

    // 可选：调整布局或显示状态提示
    if (plainTextEditVisible)
    {
        ui->statusbar->showMessage("详细信息区域已显示", 2000);
    }
    else
    {
        ui->statusbar->showMessage("详细信息区域已隐藏", 2000);
    }
}

void Copya::closeEvent(QCloseEvent *event)
{
    // 保存最后编辑的内容
    saveCurrentEdit();

    // 不关闭程序，只是隐藏窗口
    this->hide();
    event->ignore();

    // 如果需要真正退出，可以在托盘菜单中提供退出选项
    // QMainWindow::closeEvent(event);
}

void Copya::loadData()
{
    // 在清空plainTextEdit之前阻止信号
    ui->plainTextEdit->blockSignals(true);

    ui->plainTextEdit->clear();
    currentEditingId = -1;

    ui->treeWidget->clear();
    ui->treeWidget->blockSignals(true);  // 临时阻止treeWidget的信号

    auto data = database->readAllData();
    if (data.empty())
    {
        ui->treeWidget->blockSignals(false);
        ui->plainTextEdit->blockSignals(false);
        return;
    }

    // 使用反向迭代器实现倒序显示
    for (auto it = data.rbegin(); it != data.rend(); ++it)
    {
        const auto &item = *it;
        int id = std::get<0>(item);
        std::string content = std::get<1>(item);
        int mark = std::get<2>(item);

        QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget);

        // 设置内容，如果内容太长则截断
        QString contentText = QString::fromStdString(content);
        if (contentText.length() > 200)
        {
            contentText = contentText.left(200) + "...";
        }
        treeItem->setText(0, contentText);

        // 设置标记
        QString markText;
        if (mark == 1)
        {
            markText = Marks::IMPORTANTMARKTEXT;
            treeItem->setForeground(1, QBrush(Qt::red));
        }
        else if (mark == 0)
        {
            markText = Marks::NORMALMARKTEXT;
        }
        else
        {
            markText = getLabelTextByMark(mark);
        }
        treeItem->setText(1, markText);

        // 存储完整的原始内容
        treeItem->setData(0, Qt::UserRole, QString::fromStdString(content));
        treeItem->setData(0, Qt::UserRole + 1, id);

        // 根据标记设置字体
        if (mark == 1)
        {
            QFont font = treeItem->font(0);
            font.setBold(true);
            treeItem->setFont(0, font);
        }
    }

    // 自动调整列宽
    ui->treeWidget->header()->setStretchLastSection(false);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);

    ui->treeWidget->blockSignals(false);     // 恢复信号
    ui->plainTextEdit->blockSignals(false);  // 恢复信号
}
void Copya::onTreeWidgetCurrentItemChanged(QTreeWidgetItem *current,
                                           QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);

    // 在更新plainTextEdit之前，阻止信号
    ui->plainTextEdit->blockSignals(true);

    if (!current)
    {
        // 如果没有选中任何项目，清空 plainTextEdit
        ui->plainTextEdit->clear();
        currentEditingId = -1;
        ui->plainTextEdit->blockSignals(false);  // 恢复信号
        return;
    }

    // 获取新选中项目的ID和内容
    currentEditingId = current->data(0, Qt::UserRole + 1).toInt();
    QString content = current->data(0, Qt::UserRole).toString();

    // 更新 plainTextEdit 显示
    ui->plainTextEdit->setPlainText(content);

    // 恢复信号
    ui->plainTextEdit->blockSignals(false);

    qDebug() << "切换到项目，ID:" << currentEditingId;
}

void Copya::onPlainTextEditChanged()
{
    // 只有在用户实际编辑时才处理
    if (currentEditingId <= 0)
    {
        return;  // 没有有效的ID
    }

    QString newContent = ui->plainTextEdit->toPlainText();
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();

    if (!currentItem)
    {
        return;
    }

    QString oldContent = currentItem->data(0, Qt::UserRole).toString();

    // 如果内容相同，不需要更新
    if (oldContent == newContent)
    {
        return;
    }

    // 更新treeWidget中的数据（但不立即保存到数据库）
    currentItem->setData(0, Qt::UserRole, newContent);

    // 更新显示
    QString displayText = newContent;
    if (displayText.length() > 200)
    {
        displayText = displayText.left(200) + "...";
    }
    currentItem->setText(0, displayText);

    qDebug() << "内容已修改，ID:" << currentEditingId << "(未保存到数据库)";
}

void Copya::saveCurrentEdit()
{
    if (currentEditingId <= 0)
    {
        return;
    }

    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    if (!item)
    {
        return;
    }

    QString content = item->data(0, Qt::UserRole).toString();
    std::string id_str_content = database->getTextById(currentEditingId);
    QString id_content = QString::fromStdString(id_str_content);
    // 如果内容为空，则删除该项目
    if (content.trimmed().isEmpty())
    {
        if (database->deleteById(currentEditingId))
        {
            delete item;
            qDebug() << "删除空项目，ID:" << currentEditingId;
        }
        return;
    }

    // 更新数据库
    if (database && database->isOpen())
    {
        if (content != id_content)
        {
            bool success = database->updateTextById(currentEditingId,
                                                    content.toStdString());
            if (success)
            {
                qDebug() << "保存到数据库，ID:" << currentEditingId;
            }
            else
            {
                qDebug() << "保存到数据库失败，ID:" << currentEditingId;
            }
        }
    }
}
void Copya::refreshData()
{
    // 在加载数据前阻止信号
    ui->plainTextEdit->blockSignals(true);

    // 保存当前编辑的内容（如果有）
    saveCurrentEdit();

    currentEditingId = -1;
    ui->plainTextEdit->clear();

    QString searchText = ui->lineEdit->text();
    if (!searchText.isEmpty())
    {
        performSearch(searchText);
    }
    else
    {
        if (currentShowMark == -1)
        {
            loadData();
        }
        else
        {
            loadMarkData(currentShowMark);
        }
    }

    ui->plainTextEdit->blockSignals(false);
}
void Copya::onDeleteSelected()
{
    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (!currentItem)
    {
        QMessageBox::information(this, "提示", "请先选择一个项目！");
        return;
    }

    int id = currentItem->data(0, Qt::UserRole + 1).toInt();

    if (database->deleteById(id))
    {
        // 如果删除的是当前正在编辑的项目，清空编辑状态
        if (currentEditingId == id)
        {
            currentEditingId = -1;
            ui->plainTextEdit->clear();
        }

        delete currentItem;
    }
    else
    {
        QMessageBox::warning(this, "错误", "删除项目失败！");
    }
}

// void Copya::hideEvent(QHideEvent *event)
//{
//     // 保存最后编辑的内容
//     saveCurrentEdit();

//    // 不关闭程序，只是隐藏窗口
//    this->hide();
//    event->ignore();
//}

// 刷新widget_3区域的显示
void Copya::refreshLabelsDisplay()
{
    qDebug() << "开始刷新标签显示...";
    qDebug() << "ui指针:" << ui;

    // 检查 ui 和 widget_3 是否存在
    if (!ui)
    {
        qDebug() << "错误：ui 指针为空";
        return;
    }

    if (!ui->widget_3)
    {
        qDebug() << "错误：widget_3 指针为空";
        return;
    }

    qDebug() << "widget_3指针:" << ui->widget_3;

    // 清除widget_3中的所有现有widget
    QLayout *layout = ui->widget_3->layout();
    if (layout)
    {
        qDebug() << "清除现有布局...";
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != nullptr)
        {
            if (item->widget())
            {
                qDebug() << "删除widget:" << item->widget()->objectName();
                item->widget()->deleteLater();
            }
            delete item;
        }
        delete layout;
        qDebug() << "现有布局已清除";
    }
    else
    {
        qDebug() << "widget_3 没有现有布局";
    }

    // 创建新的垂直布局
    QVBoxLayout *vLayout = new QVBoxLayout(ui->widget_3);
    vLayout->setSpacing(5);
    vLayout->setContentsMargins(0, 0, 0, 0);
    qDebug() << "新布局创建完成";

    // 检查数据库是否打开
    if (!database)
    {
        qDebug() << "数据库指针为空";
        return;
    }

    if (!database->isOpen())
    {
        qDebug() << "数据库未打开";
        QLabel *infoLabel = new QLabel("数据库未连接", ui->widget_3);
        infoLabel->setAlignment(Qt::AlignCenter);
        infoLabel->setStyleSheet("color: gray; font-style: italic;");
        vLayout->addWidget(infoLabel);
        ui->widget_3->setLayout(vLayout);
        return;
    }

    // 从数据库获取所有marks和标签
    qDebug() << "从数据库获取标签数据...";
    qDebug() << "获取到" << marks.size() << "个标签";

    if (marks.empty())
    {
        qDebug() << "数据库中没有标签";
    }
    else
    {
        // 添加数据库中的标签
        for (const auto &markLabel : marks)
        {
            int mark = markLabel.first;
            QString labelText = QString::fromStdString(markLabel.second);

            qDebug() << "添加标签 - Mark:" << mark << "标签:" << labelText;

            QToolButton *button = new QToolButton(ui->widget_3);
            button->setObjectName(QString("labelButton_%1").arg(mark));

            // 设置图标
            QString iconPath = ":/main/icon/MynauiLabelSolid.svg";
            if (QFile::exists(iconPath))
            {
                button->setIcon(QIcon(iconPath));
                button->setIconSize(QSize(20, 20));
                qDebug() << "图标加载成功:" << iconPath;
            }
            else
            {
                qDebug() << "图标文件不存在:" << iconPath;
                // 如果没有图标，设置一个默认的样式或使用文字
                button->setStyleSheet(button->styleSheet() +
                                      DefaultQss::TOOLSETNULL);
            }

            button->setAutoRaise(true);
            button->setStyleSheet(button->styleSheet() + DefaultQss::TOOLSET);

            button->setProperty("mark", mark);
            button->setProperty("labelText", labelText);
            button->setToolTip(
                QString("Name: %1\n点击设置项目的标签").arg(labelText));

            // 创建右键菜单
            QMenu *contextMenu = new QMenu(button);
            QAction *editAction = contextMenu->addAction("编辑标签");
            QAction *deleteAction = contextMenu->addAction("删除标签");
            QAction *viewAction = contextMenu->addAction("查看项目");
            contextMenu->addSeparator();
            QAction *refreshAction = contextMenu->addAction("刷新显示");

            // 连接右键菜单动作
            connect(
                editAction, &QAction::triggered, this,
                [this, mark, button]()
                {
                    QString currentLabel =
                        button->property("labelText").toString();
                    bool ok;
                    QString newLabel = QInputDialog::getText(
                        this, "编辑标签",
                        "请输入新标签名称:", QLineEdit::Normal, currentLabel,
                        &ok);
                    if (ok && !newLabel.isEmpty())
                    {
                        if (database->updateLabel(mark, newLabel.toStdString()))
                        {
                            button->setText(newLabel);
                            button->setProperty("labelText", newLabel);
                            button->setToolTip(
                                QString("Name: %1\n点击设置项目的标签")
                                    .arg(newLabel));
                            QMessageBox::information(this, "成功",
                                                     "标签更新成功");
                        }
                        else
                        {
                            QMessageBox::warning(this, "错误", "标签更新失败");
                        }
                    }
                });

            connect(deleteAction, &QAction::triggered, this,
                    [this, mark, labelText]()
                    {
                        QVector<std::tuple<int, QString, int, QString>> items =
                            getDataByMark(mark);
                        int ret = QMessageBox::question(
                            this, "确认删除",
                            QString("确定要删除标签 '%1' 和相关的 %2 "
                                    "个项目吗？\n此操作不可撤销！")
                                .arg(labelText)
                                .arg(items.size()),
                            QMessageBox::Yes | QMessageBox::No);
                        if (ret == QMessageBox::Yes)
                        {
                            if (deleteMark(mark))
                            {
                                QMessageBox::information(this, "成功",
                                                         "标签删除成功");
                                refreshMarkData();
                                refreshData();
                                refreshLabelsDisplay();  // 刷新显示
                            }
                            else
                            {
                                QMessageBox::warning(this, "错误",
                                                     "标签删除失败");
                            }
                        }
                    });

            connect(viewAction, &QAction::triggered, this,
                    [this, mark, labelText]()
                    { onLabelButtonRightClicked(mark); });

            connect(refreshAction, &QAction::triggered, this,
                    [this]() { refreshLabelsDisplay(); });

            // 设置按钮的右键菜单
            button->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(button, &QToolButton::customContextMenuRequested, this,
                    [contextMenu]() { contextMenu->exec(QCursor::pos()); });

            // 连接左键点击信号
            connect(button, &QToolButton::clicked, this,
                    [this, mark]() { onMarkAsIt(mark); });

            vLayout->addWidget(button);
        }
    }

    // 添加垂直弹簧
    vLayout->addStretch();
    ui->widget_3->setLayout(vLayout);

    qDebug() << "标签显示刷新完成";
}
// 添加标签按钮的点击槽函数
void Copya::onAddLabelButtonClicked()
{
    bool ok;
    QString label = QInputDialog::getText(
        this, "添加标签", "请输入标签名称:", QLineEdit::Normal, "", &ok);

    if (ok && !label.isEmpty())
    {
        int mark = database->setLabel(label.toStdString());
        if (mark != -1)
        {
            QMessageBox::information(
                this, "成功",
                QString("标签添加成功，分配的Mark为: %1").arg(mark));
            refreshMarkData();
            refreshLabelsDisplay();  // 刷新显示
        }
        else
        {
            QMessageBox::warning(this, "错误", "标签添加失败");
        }
    }
}

// 标签右键单击槽函数
void Copya::onLabelButtonRightClicked(int mark)
{
    currentShowMark = mark;
    QString labelText = getLabelTextByMark(mark);
    qDebug() << "右键单击标签 - Mark:" << mark << "标签:" << labelText;

    // 获取mark对应的所有项目
    auto items = getDataByMark(mark);

    if (items.isEmpty())
    {
        qDebug() << "该标签下没有项目";
        QMessageBox::information(
            this, "信息", QString("标签 \"%1\" 下没有项目").arg(labelText));
    }
    else
    {
        refreshData();
    }
}

// 删除mark标识的函数
bool Copya::deleteMark(int mark)
{
    if (mark <= 0)
    {
        qDebug() << "无法删除默认mark(0)";
        return false;
    }

    // 直接调用数据库的deleteMark函数
    return database->deleteMark(mark);
}

// 查询函数，传入mark，传出为mark对应的文本，如果不存在则返回"普通"
QString Copya::getLabelTextByMark(int mark)
{
    if (mark == 0)
    {
        return Marks::NORMALMARKTEXT;
    }

    // 查找对应的mark
    for (const auto &markLabel : marks)
    {
        if (markLabel.first == mark)
        {
            return QString::fromStdString(markLabel.second);
        }
    }

    // 如果没有找到，返回"普通"
    return Marks::NORMALMARKTEXT;
}

// 获取mark对应的所有项目
QVector<std::tuple<int, QString, int, QString>> Copya::getDataByMark(int mark)
{
    // 从数据库获取数据
    std::vector<std::tuple<int, std::string, int, std::string>> dbResult =
        database->getDataByMark(mark);

    // 转换为Qt格式
    return convertToQtVector(dbResult);
}

// 辅助函数：将数据库返回的vector转换为QVector
QVector<std::tuple<int, QString, int, QString>> Copya::convertToQtVector(
    const std::vector<std::tuple<int, std::string, int, std::string>> &dbVector)
{
    QVector<std::tuple<int, QString, int, QString>> result;

    for (const auto &item : dbVector)
    {
        int id = std::get<0>(item);
        std::string text = std::get<1>(item);
        int mark = std::get<2>(item);
        std::string time = std::get<3>(item);

        result.append(std::make_tuple(id, QString::fromStdString(text), mark,
                                      QString::fromStdString(time)));
    }

    return result;
}

void Copya::loadMarkData(int mark)
{
    // 在清空plainTextEdit之前阻止信号
    ui->plainTextEdit->blockSignals(true);

    ui->plainTextEdit->clear();
    currentEditingId = -1;

    ui->treeWidget->clear();
    ui->treeWidget->blockSignals(true);  // 临时阻止treeWidget的信号

    auto data = database->getDataByMark(mark);
    if (data.empty())
    {
        ui->treeWidget->blockSignals(false);
        ui->plainTextEdit->blockSignals(false);
        return;
    }

    // 使用反向迭代器实现倒序显示
    for (auto it = data.rbegin(); it != data.rend(); ++it)
    {
        const auto &item = *it;
        int id = std::get<0>(item);
        std::string content = std::get<1>(item);
        int mark = std::get<2>(item);

        QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget);

        // 设置内容，如果内容太长则截断
        QString contentText = QString::fromStdString(content);
        if (contentText.length() > 200)
        {
            contentText = contentText.left(200) + "...";
        }
        treeItem->setText(0, contentText);

        // 设置标记
        QString markText;
        if (mark == 1)
        {
            markText = Marks::IMPORTANTMARKTEXT;
            treeItem->setForeground(1, QBrush(Qt::red));
        }
        else if (mark == 0)
        {
            markText = Marks::NORMALMARKTEXT;
        }
        else
        {
            markText = getLabelTextByMark(mark);
        }
        treeItem->setText(1, markText);

        // 存储完整的原始内容
        treeItem->setData(0, Qt::UserRole, QString::fromStdString(content));
        treeItem->setData(0, Qt::UserRole + 1, id);

        // 根据标记设置字体
        if (mark == 1)
        {
            QFont font = treeItem->font(0);
            font.setBold(true);
            treeItem->setFont(0, font);
        }
    }

    // 自动调整列宽
    ui->treeWidget->header()->setStretchLastSection(false);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);

    ui->treeWidget->blockSignals(false);     // 恢复信号
    ui->plainTextEdit->blockSignals(false);  // 恢复信号
}
