//clientwidget.cpp
#include "clientwidget.h"
#include <QPainter>

ClientWidget::ClientWidget(const QString &username, const QString &status, QWidget *parent)
    : QWidget(parent), clientUsername(username)
{
    // Initialize components
    statusCircle = new QLabel(this);
    usernameLabel = new QLabel(username, this);
    callButton = new QPushButton("Call", this);

    // Update status circle
    updateStatusCircle(status);

    // Set layout
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(statusCircle);
    mainLayout->addWidget(usernameLabel);
    mainLayout->addWidget(callButton);
    mainLayout->setSpacing(10);
    setLayout(mainLayout);

    // Connect button signal to slot
    connect(callButton, &QPushButton::clicked, this, &ClientWidget::onCallButtonClicked);
}

void ClientWidget::onCallButtonClicked()
{
    emit callButtonClicked(clientUsername);
}

void ClientWidget::updateStatusCircle(const QString &status)
{
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor color;
    if (status == "Online")
        color = Qt::green;
    else if (status == "Offline")
        color = Qt::red;
    else if (status == "Busy")
        color = Qt::yellow;
    else
        color = Qt::gray;

    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 20, 20);
    statusCircle->setPixmap(pixmap);
}
