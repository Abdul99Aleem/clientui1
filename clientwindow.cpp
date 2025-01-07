//clientwindow.cpp
#include "clientwindow.h"
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
#include <QSlider>
#include "conferancecallwindow.h"
#include <QCloseEvent>
#include <QApplication>
#include <QMessageBox>
#include "clientwidget.h"
#include <QStandardPaths>
#include <QSettings>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

ClientWindow::ClientWindow(MainWindow *mainwindow, QWidget *parent)
    : QMainWindow(parent), mainWindow(mainwindow)
{
    mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);

    // Initialize layouts
    topBox = new QHBoxLayout();
    clientBox = new QHBoxLayout();
    makeConfCallLayout = new QHBoxLayout();
    MidBox = new QHBoxLayout();
    mainLayout = new QVBoxLayout();

    // Initialize stacked widget for main content
    mainStack = new QStackedWidget(this);

    // Create call state widgets
    setupCallLayouts();

    // Volume slider setup
    volumeSlider = new QSlider(Qt::Horizontal, this);  // Change to member variable
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    volumeSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    volumeSlider->setTickInterval(10);
    volumeSlider->setTickPosition(QSlider::TicksBelow);
    connect(volumeSlider, &QSlider::valueChanged, this, &ClientWindow::handleVolumeChange);

    // Top box setup with proper initial connection status
    clientStatusCircle = new QLabel(this);
    clientStatusCircle->setFixedSize(25, 25);
    clientStatusCircle->setStyleSheet("border-radius: 12px; background-color: red;");
    clientName = new QLabel("Not Connected to Server", this);
    themeBtn = new QPushButton("Dark/Light Mode", this);
    exitBtn = new QPushButton("Exit", this);
    logout = new QPushButton("Logout", this);

    topBox->addWidget(clientStatusCircle, 1);
    topBox->addWidget(clientName, 1);
    topBox->addWidget(themeBtn, 1);
    topBox->addWidget(exitBtn, 1);
    topBox->addWidget(volumeSlider, 1);
    topBox->addWidget(logout, 1);

    // Client list setup
    clientList = new QListWidget(this);

    // Create left panel
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->addWidget(clientList);

    // Create right panel
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->addWidget(mainStack);

    // Setup conference UI
    setupConferenceUI();

    // Main layout setup
    MidBox->addWidget(leftPanel, 1);
    MidBox->addWidget(rightPanel, 2);

    mainLayout->addLayout(topBox);
    mainLayout->addLayout(MidBox);
    mainLayout->addLayout(makeConfCallLayout);
    mainWidget->setLayout(mainLayout);

    // Connect signals and apply theme
    connectSignals();
    loadThemePreference();
    applyTheme(isDarkTheme);

    // Set layout properties
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);
    clientList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Set button properties
    themeBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    exitBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    logout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    themeBtn->setMinimumHeight(40);
    exitBtn->setMinimumHeight(40);
    logout->setMinimumHeight(40);
}



void ClientWindow::setupCallLayouts() {
    // No call layout
    noCallWidget = new QWidget(this);
    noCallLayout = new QVBoxLayout(noCallWidget);
    noCallLabel = new QLabel("NO active calls", this);
    noCallLayout->addWidget(noCallLabel);
    mainStack->addWidget(noCallWidget);

    // Incoming call layout
    incomingCallWidget = new QWidget(this);
    incomingCallLayout = new QVBoxLayout(incomingCallWidget);
    incomingCallLabel = new QLabel("Incoming call....", this);
    incomingClientLabel = new QLabel("Client Name", this);
    acceptCall_btn = new QPushButton("Accept Call", this);
    rejectCall_btn = new QPushButton("Reject Call", this);
    incomingCallLayout->addWidget(incomingCallLabel);
    incomingCallLayout->addWidget(incomingClientLabel);
    incomingCallLayout->addWidget(acceptCall_btn);
    incomingCallLayout->addWidget(rejectCall_btn);
    mainStack->addWidget(incomingCallWidget);

    // Ongoing call layout
    ongoingCallWidget = new QWidget(this);
    ongoingCallLayout = new QVBoxLayout(ongoingCallWidget);
    ongoingClientLabel = new QLabel("Client Name", this);
    leaveCall_btn = new QPushButton("End Call", this);
    ongoingCallLayout->addWidget(ongoingClientLabel);
    ongoingCallLayout->addWidget(leaveCall_btn);
    mainStack->addWidget(ongoingCallWidget);

    // Outgoing call layout
    outgoingCallWidget = new QWidget(this);
    outgoingCallLayout = new QVBoxLayout(outgoingCallWidget);
    outgoingCallLabel = new QLabel("Ringing....", this);
    outgoingClientLabel = new QLabel("Client Name", this);
    endCall_btn = new QPushButton("End Call", this);
    outgoingCallLayout->addWidget(outgoingCallLabel);
    outgoingCallLayout->addWidget(outgoingClientLabel);
    outgoingCallLayout->addWidget(endCall_btn);
    mainStack->addWidget(outgoingCallWidget);

    // Set initial widget
    mainStack->setCurrentWidget(noCallWidget);
}

void ClientWindow::populateList() {
    for (const auto &client : clients) {
        QWidget *clientWidget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(clientWidget);
        layout->setSpacing(10);
        layout->setContentsMargins(5, 5, 5, 5);

        QLabel *status = new QLabel(clientWidget);
        status->setPixmap(getStatusIcon(client.status));
        status->setFixedSize(16, 16);

        QLabel *username = new QLabel(client.username, clientWidget);
        username->setMinimumWidth(100);

        QCheckBox *selectBox = new QCheckBox(clientWidget);
        selectBox->hide();

        QPushButton *msgBtn = new QPushButton("Msg", clientWidget);
        msgBtn->setFixedWidth(60);
        connect(msgBtn, &QPushButton::clicked, this, [this, username]() {
            showMessageScreen(username->text());
        });

        QPushButton *callBtn = new QPushButton("Call", clientWidget);
        callBtn->setFixedWidth(60);
        connect(callBtn, &QPushButton::clicked, this, [this, username]() {
            currentClient = username->text();
            onCallBtnClicked();
        });

        layout->addWidget(status);
        layout->addWidget(username, 1);
        layout->addWidget(selectBox);
        layout->addWidget(msgBtn);
        layout->addWidget(callBtn);
        layout->addStretch();

        QListWidgetItem *item = new QListWidgetItem(clientList);
        item->setSizeHint(clientWidget->sizeHint());
        clientList->setItemWidget(item, clientWidget);
    }
}
void ClientWindow::showMessageScreen(const QString &username) {
    if (!messageWindows.contains(username)) {
        MessageWindow *window = new MessageWindow(username, isDarkTheme, this);
        messageWindows[username] = window;
        mainStack->addWidget(window);

        connect(window, &MessageWindow::backButtonClicked, this, &ClientWindow::showHomeScreen);
        connect(window, &MessageWindow::closed, this, [this, username]() {
            removeMessageWindow(username);
        });
    }
    mainStack->setCurrentWidget(messageWindows[username]);
}

void ClientWindow::showHomeScreen() {
    mainStack->setCurrentWidget(noCallWidget);
}

void ClientWindow::removeMessageWindow(const QString &username) {
    if (messageWindows.contains(username)) {
        MessageWindow *window = messageWindows[username];
        messageWindows.remove(username);
        mainStack->removeWidget(window);
        window->deleteLater();
    }
}

void ClientWindow::switchToLayout(int index) {
    switch(index) {
    case 0: // No call
        mainStack->setCurrentWidget(noCallWidget);
        break;
    case 1: // Incoming call
        mainStack->setCurrentWidget(incomingCallWidget);
        break;
    case 2: // Ongoing call
        mainStack->setCurrentWidget(ongoingCallWidget);
        break;
    case 3: // Outgoing call
        mainStack->setCurrentWidget(outgoingCallWidget);
        break;
    }
}

void ClientWindow::setupConferenceUI() {
    QPushButton *conferenceToggle = new QPushButton("Conference Mode", this);
    makeConfCallLayout->addWidget(conferenceToggle);

    conferencePanel = new QWidget(this);
    QVBoxLayout *confLayout = new QVBoxLayout(conferencePanel);
    mainLayout->addWidget(conferencePanel);

    selectAllCheckbox = new QCheckBox("Select All", conferencePanel);
    confLayout->addWidget(selectAllCheckbox);

    startConferenceBtn = new QPushButton("Start Conference", conferencePanel);
    confLayout->addWidget(startConferenceBtn);

    conferencePanel->hide();

    connect(conferenceToggle, &QPushButton::clicked, this, &ClientWindow::toggleConferenceMode);
    connect(selectAllCheckbox, &QCheckBox::checkStateChanged, this, &ClientWindow::handleSelectAll);
    connect(startConferenceBtn, &QPushButton::clicked, this, &ClientWindow::startConference);
}

void ClientWindow::handleSelectAll(Qt::CheckState state) {
    for (int i = 0; i < clientList->count(); i++) {
        QWidget *widget = clientList->itemWidget(clientList->item(i));
        QCheckBox *checkbox = widget->findChild<QCheckBox*>();
        if (checkbox) {
            checkbox->setCheckState(state);
        }
    }
}

void ClientWindow::toggleConferenceMode() {
    bool isConferenceMode = !conferencePanel->isVisible();
    conferencePanel->setVisible(isConferenceMode);

    for (int i = 0; i < clientList->count(); i++) {
        QWidget *widget = clientList->itemWidget(clientList->item(i));
        widget->findChild<QCheckBox*>()->setVisible(isConferenceMode);
    }
}

void ClientWindow::startConference() {
    QList<QString> selectedClients;

    for (int i = 0; i < clientList->count(); i++) {
        QWidget *widget = clientList->itemWidget(clientList->item(i));
        QCheckBox *checkbox = widget->findChild<QCheckBox*>();
        if (checkbox && checkbox->isChecked()) {
            QLabel *username = widget->findChild<QLabel*>();
            selectedClients.append(username->text());
        }
    }

    if (selectedClients.size() < 2) {
        QMessageBox::warning(this, "Conference Call",
                             "Please select at least 2 participants");
        return;
    }

    switchToLayout(2);
    ongoingClientLabel->setText("Conference Call: " + selectedClients.join(", "));
    conferencePanel->hide();
    selectAllCheckbox->setChecked(false);
    handleSelectAll(Qt::Unchecked);
}

void ClientWindow::connectSignals() {
    connect(acceptCall_btn, &QPushButton::clicked, this, &ClientWindow::onOngoingCall);
    connect(leaveCall_btn, &QPushButton::clicked, [this]() { switchToLayout(0); });
    connect(endCall_btn, &QPushButton::clicked, [this]() { switchToLayout(0); });
    connect(rejectCall_btn, &QPushButton::clicked, [this]() { switchToLayout(0); });
    connect(logout, &QPushButton::clicked, this, &ClientWindow::onLogoutBtnClicked);
    connect(exitBtn, &QPushButton::clicked, this, &ClientWindow::onExitBtnClicked);
    connect(themeBtn, &QPushButton::clicked, this, &ClientWindow::toggleTheme);
}

void ClientWindow::toggleTheme() {
    isDarkTheme = !isDarkTheme;
    applyTheme(isDarkTheme);
    saveThemePreference();

    // Update theme for all message windows
    for (auto window : messageWindows) {
        window->updateTheme(isDarkTheme);
    }
}

void ClientWindow::applyTheme(bool isDark) {
    QString styleSheet = isDark ? getDarkThemeStyleSheet() : getLightThemeStyleSheet();
    mainWidget->setStyleSheet(styleSheet);
    QString statusCircleStyle = QString("border-radius: 12px; background-color: %1;")
                                    .arg(isDark ? "#404040" : "#e0e0e0");
    clientStatusCircle->setStyleSheet(statusCircleStyle);
}

void ClientWindow::saveThemePreference() {
    QSettings settings("YourCompany", "VoIPClient");
    settings.setValue("darkTheme", isDarkTheme);
}

void ClientWindow::loadThemePreference() {
    QSettings settings("YourCompany", "VoIPClient");
    isDarkTheme = settings.value("darkTheme", false).toBool();
}

QPixmap ClientWindow::getStatusIcon(const QString &status) {
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    if (status == "Online")
        painter.setBrush(Qt::green);
    else if (status == "Offline")
        painter.setBrush(Qt::red);
    else if (status == "Busy")
        painter.setBrush(Qt::yellow);
    else
        painter.setBrush(Qt::gray);

    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 16, 16);
    return pixmap;
}

void ClientWindow::handleIncomingCall(const QString &caller) {
    currentClient = caller;
    incomingClientLabel->setText(caller);
    switchToLayout(1);
}

void ClientWindow::onCallAccepted() {
    ongoingClientLabel->setText(currentClient);
    switchToLayout(2);
}

void ClientWindow::onCallRejected() {
    switchToLayout(0);
    currentClient.clear();
}

void ClientWindow::onCallBtnClicked() {
    if (!currentClient.isEmpty()) {
        outgoingClientLabel->setText(currentClient);
        switchToLayout(3);
    }
}

void ClientWindow::onLogoutBtnClicked() {
    if (mainWindow) {
        if (!mainWindow->rememberMe->isChecked()) {
            mainWindow->clearCredentials();
        }
        mainWindow->show();
    }
    this->close();
}

void ClientWindow::onExitBtnClicked() {
    QApplication::quit();
}

QString ClientWindow::getDarkThemeStyleSheet()
{
    return R"(
        QWidget {
            background-color: #1E2922;
            color: #E0E3DE;
        }
        QPushButton {
            background-color: #3F4A3C;
            border: 1px solid #2F3E2C;
            padding: 5px;
            border-radius: 4px;
            color: white;
        }
        QListWidget {
            background-color: #2A362F;
            border: 1px solid #3F4A3C;
        }
    )";
}

QString ClientWindow::getLightThemeStyleSheet()
{
    return R"(
        QWidget {
            background-color: #E0E3DE;
            color: #2F3E2C;
        }
        QPushButton {
            background-color: #4A5D45;
            border: 1px solid #2F3E2C;
            padding: 5px;
            border-radius: 4px;
            color: white;
        }
        QListWidget {
            background-color: #FFFFFF;
            border: 1px solid #A3A69F;
        }
    )";
}

void ClientWindow::onOngoingCall()
{
    switchToLayout(2);
    ongoingClientLabel->setText(currentClient);
}

void ClientWindow::handleServerUpdate(const QByteArray &data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
        QJsonArray clientArray = doc.array();
        clientList->clear();

        for (const QJsonValue &value : clientArray) {
            QJsonObject clientObj = value.toObject();
            QString clientName = clientObj["name"].toString();
            QString status = clientObj["status"].toString(); // "Online", "Offline", "Busy"

            QWidget *clientWidget = new ClientWidget(clientName, status, this);
            QListWidgetItem *item = new QListWidgetItem(clientList);
            item->setSizeHint(clientWidget->sizeHint());
            clientList->setItemWidget(item, clientWidget);
        }
    }
}

void ClientWindow::handleVolumeChange(int value) {
    QString command = QString("amixer -c 0 sset Master %1%").arg(value);
    system(command.toStdString().c_str());
}



ClientWindow::~ClientWindow() {
    // Clean up message windows
    for (auto window : messageWindows) {
        window->deleteLater();
    }
    messageWindows.clear();

    // Clean up other widgets
    delete mainWidget;
    delete clientStatusCircle;
    delete clientName;
    delete themeBtn;
    delete exitBtn;
    delete logout;
    delete clientList;
    delete conferencePanel;
    delete selectAllCheckbox;
    delete startConferenceBtn;
}
