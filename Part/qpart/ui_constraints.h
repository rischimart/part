/********************************************************************************
** Form generated from reading UI file 'constraints.ui'
**
** Created
**      by: Qt User Interface Compiler version 4.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONSTRAINTS_H
#define UI_CONSTRAINTS_H

#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>

QT_BEGIN_NAMESPACE

class Ui_Constraints
{
public:
    QLabel *TextLabel1;
    QPushButton *constraintsCancel;
    QPushButton *constraintsOK;
    QSlider *Slider1;
    QLCDNumber *LCDNumber1;

    void setupUi(QDialog *Constraints)
    {
        if (Constraints->objectName().isEmpty())
            Constraints->setObjectName(QString::fromUtf8("Constraints"));
        Constraints->resize(344, 105);
        TextLabel1 = new QLabel(Constraints);
        TextLabel1->setObjectName(QString::fromUtf8("TextLabel1"));
        TextLabel1->setGeometry(QRect(10, 40, 120, 20));
        TextLabel1->setWordWrap(false);
        constraintsCancel = new QPushButton(Constraints);
        constraintsCancel->setObjectName(QString::fromUtf8("constraintsCancel"));
        constraintsCancel->setGeometry(QRect(270, 60, 60, 32));
        constraintsOK = new QPushButton(Constraints);
        constraintsOK->setObjectName(QString::fromUtf8("constraintsOK"));
        constraintsOK->setGeometry(QRect(270, 20, 60, 30));
        Slider1 = new QSlider(Constraints);
        Slider1->setObjectName(QString::fromUtf8("Slider1"));
        Slider1->setGeometry(QRect(20, 10, 220, 16));
        Slider1->setValue(20);
        Slider1->setOrientation(Qt::Horizontal);
        LCDNumber1 = new QLCDNumber(Constraints);
        LCDNumber1->setObjectName(QString::fromUtf8("LCDNumber1"));
        LCDNumber1->setGeometry(QRect(160, 40, 64, 23));
        LCDNumber1->setSegmentStyle(QLCDNumber::Filled);
        LCDNumber1->setProperty("value", QVariant(20));

        retranslateUi(Constraints);
        QObject::connect(constraintsCancel, SIGNAL(clicked()), Constraints, SLOT(reject()));
        QObject::connect(Slider1, SIGNAL(valueChanged(int)), LCDNumber1, SLOT(display(int)));
        QObject::connect(constraintsOK, SIGNAL(pressed()), Constraints, SLOT(accept()));
        QObject::connect(Slider1, SIGNAL(valueChanged(int)), Constraints, SLOT(Slider1_valueChanged(int)));

        QMetaObject::connectSlotsByName(Constraints);
    } // setupUi

    void retranslateUi(QDialog *Constraints)
    {
        Constraints->setWindowTitle(QApplication::translate("Constraints", "Constraints", 0, QApplication::UnicodeUTF8));
        TextLabel1->setText(QApplication::translate("Constraints", "Balance Tolerance", 0, QApplication::UnicodeUTF8));
        constraintsCancel->setText(QApplication::translate("Constraints", "&Cancel", 0, QApplication::UnicodeUTF8));
        constraintsOK->setText(QApplication::translate("Constraints", "&OK", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Constraints: public Ui_Constraints {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONSTRAINTS_H
