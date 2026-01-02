#include "copya.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>

#include "./ui_copya.h"

Copya::Copya(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::Copya),
      database(nullptr),
      plainTextEditVisible(true),
      currentEditingId(-1),
      isManualEdit(false)
{
    ui->setupUi(this);
}

void Copya::init()
{
    // 初始化数据库
    database = new ClipDatabase();
    monitor = new Monitor();

    monitor->start();

    if (!database->isOpen())
    {
        QMessageBox::critical(this, "错误", "无法打开数据库！");
        return;
    }

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
    connect(ui->searchButton, &QToolButton::clicked, this,
            &Copya::onSearchButtonClicked);
    // 添加 plainTextEdit 内容改变的信号连接
    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this,
            &Copya::onPlainTextEditChanged);

    // 初始化时隐藏 plainTextEdit（可选）
    ui->plainTextEdit->setVisible(true);

    // 连接树形控件当前项目改变的信号
    connect(ui->treeWidget, &QTreeWidget::currentItemChanged, this,
            &Copya::onTreeWidgetCurrentItemChanged);

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
        currentItem->setText(1, "重要");
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
        currentItem->setText(1, "普通");
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

// 搜索按钮点击
void Copya::onSearchButtonClicked()
{
    QString searchText = ui->lineEdit->text();
    if (!searchText.isEmpty())
    {
        performSearch(searchText);
    }
}

// 清除搜索
void Copya::onClearSearchClicked()
{
    ui->lineEdit->clear();
    refreshData();
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
    if (!database->isOpen())
    {
        QMessageBox::warning(this, "错误", "数据库未打开！");
        return;
    }

    if (searchText.isEmpty())
    {
        loadData();  // 加载所有数据
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
            markText = "重要";
            treeItem->setForeground(1, QBrush(Qt::red));
        }
        else if (mark == 0)
        {
            markText = "普通";
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
    isManualEdit = true;
    ui->plainTextEdit->clear();
    isManualEdit = false;
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

        // 添加 F4 键处理
        if (keyEvent->key() == Qt::Key_F4)
        {
            togglePlainTextEditVisibility();
            return true;
        }
    }

    // 添加 ESC 键隐藏窗口的处理（全局）
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            this->hide();
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
            markText = "重要";
            treeItem->setForeground(1, QBrush(Qt::red));
        }
        else if (mark == 0)
        {
            markText = "普通";
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

    // 保存之前编辑项目的内容（如果有）
    // 注意：这里应该保存previous项目的内容，而不是当前项目
    if (previous && currentEditingId > 0)
    {
        // 从previous项目获取内容，而不是从plainTextEdit
        QString previousContent = previous->data(0, Qt::UserRole).toString();
        QString previousDisplayText = previous->text(0);

        // 更新数据库
        if (database && database->isOpen() &&
            !previousContent.trimmed().isEmpty())
        {
            bool success = database->updateTextById(
                currentEditingId, previousContent.toStdString());
            if (success)
            {
                qDebug() << "保存成功，ID:" << currentEditingId;
            }
            else
            {
                qDebug() << "保存失败，ID:" << currentEditingId;
            }
        }
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

// 修改 saveCurrentEdit 函数，只在需要时调用
void Copya::saveCurrentEdit()
{
    // 这个函数现在只在特殊情况下调用（如删除、刷新、关闭等）
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

    // 更新数据库
    if (database && database->isOpen() && !content.trimmed().isEmpty())
    {
        bool success =
            database->updateTextById(currentEditingId, content.toStdString());
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

// 修改 refreshData 函数
void Copya::refreshData()
{
    // 在加载数据前阻止信号
    ui->plainTextEdit->blockSignals(true);

    // 保存当前编辑的内容（如果有）
    if (currentEditingId > 0)
    {
        QTreeWidgetItem *item = ui->treeWidget->currentItem();
        if (item)
        {
            QString content = item->data(0, Qt::UserRole).toString();
            if (database && database->isOpen() && !content.trimmed().isEmpty())
            {
                database->updateTextById(currentEditingId,
                                         content.toStdString());
            }
        }
    }

    currentEditingId = -1;
    ui->plainTextEdit->clear();

    QString searchText = ui->lineEdit->text();
    if (!searchText.isEmpty())
    {
        performSearch(searchText);
    }
    else
    {
        loadData();
    }

    ui->plainTextEdit->blockSignals(false);
}

// 修改 onDeleteSelected 函数
void Copya::onDeleteSelected()
{
    // 清空当前编辑状态
    currentEditingId = -1;
    ui->plainTextEdit->clear();

    QTreeWidgetItem *currentItem = ui->treeWidget->currentItem();
    if (!currentItem)
    {
        QMessageBox::information(this, "提示", "请先选择一个项目！");
        return;
    }

    int id = currentItem->data(0, Qt::UserRole + 1).toInt();

    int ret = QMessageBox::question(this, "确认", "确定要删除选中的项目吗？",
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes)
    {
        if (database->deleteById(id))
        {
            delete currentItem;
            QMessageBox::information(this, "成功", "项目已删除！");
        }
        else
        {
            QMessageBox::warning(this, "错误", "删除项目失败！");
        }
    }
}

void Copya::hideEvent(QHideEvent *event)
{
    // TODO:似乎不起作用
    //  保存最后编辑的内容
    saveCurrentEdit();

    // 不关闭程序，只是隐藏窗口
    this->hide();
    event->ignore();
}
