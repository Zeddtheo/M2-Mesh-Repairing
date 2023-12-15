/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.6.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <meshviewerwidget.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_6;
    QWidget *widget_2;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QPushButton *pushButton_chargement;
    QPushButton *pushButton_Components;
    QPushButton *pushButton_FixAll;
    QPushButton *pushButton_Reset;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_Holes;
    QPushButton *pushButton_FixHoles;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *pushButton_Cracks;
    QPushButton *pushButton_FixCracks;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *pushButton_Noises;
    QPushButton *pushButton_FixNoises;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *pushButton_Floaters;
    QPushButton *pushButton_FixFloaters;
    MeshViewerWidget *displayWidget;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(673, 457);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        horizontalLayout_6 = new QHBoxLayout(centralWidget);
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        widget_2 = new QWidget(centralWidget);
        widget_2->setObjectName("widget_2");
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(150);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(widget_2->sizePolicy().hasHeightForWidth());
        widget_2->setSizePolicy(sizePolicy);
        widget_2->setMinimumSize(QSize(150, 0));
        gridLayout = new QGridLayout(widget_2);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName("gridLayout");
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName("verticalLayout_2");
        pushButton_chargement = new QPushButton(widget_2);
        pushButton_chargement->setObjectName("pushButton_chargement");
        pushButton_chargement->setMinimumSize(QSize(200, 0));

        verticalLayout_2->addWidget(pushButton_chargement);

        pushButton_Components = new QPushButton(widget_2);
        pushButton_Components->setObjectName("pushButton_Components");

        verticalLayout_2->addWidget(pushButton_Components);

        pushButton_FixAll = new QPushButton(widget_2);
        pushButton_FixAll->setObjectName("pushButton_FixAll");

        verticalLayout_2->addWidget(pushButton_FixAll);

        pushButton_Reset = new QPushButton(widget_2);
        pushButton_Reset->setObjectName("pushButton_Reset");

        verticalLayout_2->addWidget(pushButton_Reset);


        verticalLayout_3->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName("horizontalLayout");
        pushButton_Holes = new QPushButton(widget_2);
        pushButton_Holes->setObjectName("pushButton_Holes");

        horizontalLayout->addWidget(pushButton_Holes);

        pushButton_FixHoles = new QPushButton(widget_2);
        pushButton_FixHoles->setObjectName("pushButton_FixHoles");

        horizontalLayout->addWidget(pushButton_FixHoles);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        pushButton_Cracks = new QPushButton(widget_2);
        pushButton_Cracks->setObjectName("pushButton_Cracks");

        horizontalLayout_3->addWidget(pushButton_Cracks);

        pushButton_FixCracks = new QPushButton(widget_2);
        pushButton_FixCracks->setObjectName("pushButton_FixCracks");

        horizontalLayout_3->addWidget(pushButton_FixCracks);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        pushButton_Noises = new QPushButton(widget_2);
        pushButton_Noises->setObjectName("pushButton_Noises");

        horizontalLayout_4->addWidget(pushButton_Noises);

        pushButton_FixNoises = new QPushButton(widget_2);
        pushButton_FixNoises->setObjectName("pushButton_FixNoises");

        horizontalLayout_4->addWidget(pushButton_FixNoises);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        pushButton_Floaters = new QPushButton(widget_2);
        pushButton_Floaters->setObjectName("pushButton_Floaters");

        horizontalLayout_5->addWidget(pushButton_Floaters);

        pushButton_FixFloaters = new QPushButton(widget_2);
        pushButton_FixFloaters->setObjectName("pushButton_FixFloaters");

        horizontalLayout_5->addWidget(pushButton_FixFloaters);


        verticalLayout->addLayout(horizontalLayout_5);


        verticalLayout_3->addLayout(verticalLayout);


        gridLayout->addLayout(verticalLayout_3, 0, 0, 1, 1);


        horizontalLayout_6->addWidget(widget_2);

        displayWidget = new MeshViewerWidget(centralWidget);
        displayWidget->setObjectName("displayWidget");

        horizontalLayout_6->addWidget(displayWidget);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        pushButton_chargement->setText(QCoreApplication::translate("MainWindow", "Charger OBJ", nullptr));
        pushButton_Components->setText(QCoreApplication::translate("MainWindow", "Show Components", nullptr));
        pushButton_FixAll->setText(QCoreApplication::translate("MainWindow", "Fix All", nullptr));
        pushButton_Reset->setText(QCoreApplication::translate("MainWindow", "Reset", nullptr));
        pushButton_Holes->setText(QCoreApplication::translate("MainWindow", "Holes", nullptr));
        pushButton_FixHoles->setText(QCoreApplication::translate("MainWindow", "Fix Holes", nullptr));
        pushButton_Cracks->setText(QCoreApplication::translate("MainWindow", "Cracks", nullptr));
        pushButton_FixCracks->setText(QCoreApplication::translate("MainWindow", "Fix Cracks", nullptr));
        pushButton_Noises->setText(QCoreApplication::translate("MainWindow", "Noises", nullptr));
        pushButton_FixNoises->setText(QCoreApplication::translate("MainWindow", "Fix Noises", nullptr));
        pushButton_Floaters->setText(QCoreApplication::translate("MainWindow", "Floaters", nullptr));
        pushButton_FixFloaters->setText(QCoreApplication::translate("MainWindow", "Fix Floaters", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
