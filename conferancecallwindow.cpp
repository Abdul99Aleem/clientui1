//conferancecallwindow.cpp
#include "conferancecallwindow.h"
#include <QObject>
#include <QWidget>
#include <QMainWindow>
#include <QLayout>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QComboBox>
#include <QDebug>
#include <QInputDialog>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QProcess>
#include "mainwindow.h"
#include "clientwindow.h"
#include <QSlider>
#include <QCloseEvent>
#include <QApplication>
#include <QMessageBox>

ConferanceCallWindow::ConferanceCallWindow(QWidget *parent)
    : QMainWindow (parent)
{
    QStringList clients = {"Client 1", "Client 2", "Client 3"};

    //layouts
    ConfClientListLayout = new QVBoxLayout(this);
    MakeCallLayout = new QHBoxLayout(this);
    ConfWinMainLayout = new QVBoxLayout(this);

    createClientCheckboxes(clients);//creates checkboxes for every client in clients

    //button layout
    StartCall_btn = new QPushButton("Start call",this);
    MakeCallLayout -> addWidget(StartCall_btn);


    ConfWinMainLayout -> addLayout(ConfClientListLayout);
    ConfWinMainLayout -> addLayout(MakeCallLayout);


    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(ConfWinMainLayout);
    setCentralWidget(centralWidget);

}

void ConferanceCallWindow::createClientCheckboxes(const QStringList &clients)
{
    for (const QString &client : clients) {
        QCheckBox *checkbox = new QCheckBox(client, this); // Create a checkbox for each client
        checkboxes.append(checkbox);
        ConfClientListLayout->addWidget(checkbox); // Add checkbox to layout
    }
}




void ConferanceCallWindow::onStartCallBtnClicked() {
    // Emit signal to notify Linphone that the button was clicked
    emit startCallClicked();
}
