/********************************************************************************
** Form generated from reading UI file 'copya.ui'
**
** Created by: Qt User Interface Compiler version 5.15.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COPYA_H
#define UI_COPYA_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>
#include "customtreewidget.h"

QT_BEGIN_NAMESPACE

class Ui_Copya
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout_3;
    QWidget *widget_2;
    QGridLayout *gridLayout_2;
    CustomTreeWidget *treeWidget;
    QPlainTextEdit *plainTextEdit;
    QToolButton *searchButton;
    QLineEdit *lineEdit;
    QWidget *widget;
    QGridLayout *gridLayout;
    QWidget *widget_3;
    QToolButton *markNormalButton;
    QSpacerItem *verticalSpacer;
    QToolButton *deleteButton;
    QToolButton *refreshButton;
    QToolButton *markImportantButton;
    QToolButton *addLabelButton;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *Copya)
    {
        if (Copya->objectName().isEmpty())
            Copya->setObjectName(QString::fromUtf8("Copya"));
        Copya->resize(568, 375);
        centralwidget = new QWidget(Copya);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout_3 = new QGridLayout(centralwidget);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        widget_2 = new QWidget(centralwidget);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        gridLayout_2 = new QGridLayout(widget_2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        treeWidget = new CustomTreeWidget(widget_2);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidget->setHeaderItem(__qtreewidgetitem);
        treeWidget->setObjectName(QString::fromUtf8("treeWidget"));

        gridLayout_2->addWidget(treeWidget, 0, 0, 2, 1);

        plainTextEdit = new QPlainTextEdit(widget_2);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));

        gridLayout_2->addWidget(plainTextEdit, 0, 1, 2, 1);


        gridLayout_3->addWidget(widget_2, 1, 0, 2, 2);

        searchButton = new QToolButton(centralwidget);
        searchButton->setObjectName(QString::fromUtf8("searchButton"));

        gridLayout_3->addWidget(searchButton, 0, 1, 1, 1);

        lineEdit = new QLineEdit(centralwidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        gridLayout_3->addWidget(lineEdit, 0, 0, 1, 1);

        widget = new QWidget(centralwidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        gridLayout = new QGridLayout(widget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        widget_3 = new QWidget(widget);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));

        gridLayout->addWidget(widget_3, 4, 0, 1, 1);

        markNormalButton = new QToolButton(widget);
        markNormalButton->setObjectName(QString::fromUtf8("markNormalButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/main/icon/MynauiLabelLight.svg"), QSize(), QIcon::Normal, QIcon::Off);
        markNormalButton->setIcon(icon);
        markNormalButton->setIconSize(QSize(20, 20));

        gridLayout->addWidget(markNormalButton, 3, 0, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 6, 0, 1, 1);

        deleteButton = new QToolButton(widget);
        deleteButton->setObjectName(QString::fromUtf8("deleteButton"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/main/icon/MaterialSymbolsDelete.svg"), QSize(), QIcon::Normal, QIcon::Off);
        deleteButton->setIcon(icon1);
        deleteButton->setIconSize(QSize(20, 20));

        gridLayout->addWidget(deleteButton, 1, 0, 1, 1);

        refreshButton = new QToolButton(widget);
        refreshButton->setObjectName(QString::fromUtf8("refreshButton"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/main/icon/SolarRefreshBold.svg"), QSize(), QIcon::Normal, QIcon::Off);
        refreshButton->setIcon(icon2);
        refreshButton->setIconSize(QSize(20, 20));

        gridLayout->addWidget(refreshButton, 0, 0, 1, 1);

        markImportantButton = new QToolButton(widget);
        markImportantButton->setObjectName(QString::fromUtf8("markImportantButton"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/main/icon/MynauiLabelSolid.svg"), QSize(), QIcon::Normal, QIcon::Off);
        markImportantButton->setIcon(icon3);
        markImportantButton->setIconSize(QSize(20, 20));

        gridLayout->addWidget(markImportantButton, 2, 0, 1, 1);

        addLabelButton = new QToolButton(widget);
        addLabelButton->setObjectName(QString::fromUtf8("addLabelButton"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/main/icon/MaterialSymbolsAdd.svg"), QSize(), QIcon::Normal, QIcon::Off);
        addLabelButton->setIcon(icon4);
        addLabelButton->setIconSize(QSize(20, 20));

        gridLayout->addWidget(addLabelButton, 5, 0, 1, 1);


        gridLayout_3->addWidget(widget, 0, 2, 3, 1);

        Copya->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Copya);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 568, 32));
        Copya->setMenuBar(menubar);
        statusbar = new QStatusBar(Copya);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        Copya->setStatusBar(statusbar);

        retranslateUi(Copya);

        QMetaObject::connectSlotsByName(Copya);
    } // setupUi

    void retranslateUi(QMainWindow *Copya)
    {
        Copya->setWindowTitle(QCoreApplication::translate("Copya", "Copya", nullptr));
        searchButton->setText(QCoreApplication::translate("Copya", "...", nullptr));
        markNormalButton->setText(QCoreApplication::translate("Copya", "...", nullptr));
        deleteButton->setText(QCoreApplication::translate("Copya", "...", nullptr));
        refreshButton->setText(QCoreApplication::translate("Copya", "...", nullptr));
        markImportantButton->setText(QCoreApplication::translate("Copya", "...", nullptr));
        addLabelButton->setText(QCoreApplication::translate("Copya", "...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Copya: public Ui_Copya {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COPYA_H
