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

ClientWindow::ClientWindow(MainWindow *mainwindow, QWidget *parent)
    : QMainWindow(parent), mainWindow(mainwindow)
{
    mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);

    // Initialize layouts
    topBox = new QHBoxLayout();
    clientBox = new QHBoxLayout();
    makeConfCallLayout = new QHBoxLayout();
    incomingCallLayout = new QVBoxLayout();
    ongoingCallLayout = new QVBoxLayout();
    noCallLayout = new QVBoxLayout();
    outgoingCallLayout = new QVBoxLayout();
    MidBox = new QHBoxLayout();
    mainLayout = new QVBoxLayout();

    // Create widgets for different call states
    noCallWidget = new QWidget(this);
    incomingCallWidget = new QWidget(this);
    ongoingCallWidget = new QWidget(this);
    outgoingCallWidget = new QWidget(this);

    // Volume slider setup
    QSlider *volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    volumeSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    volumeSlider->setTickInterval(10);
    volumeSlider->setTickPosition(QSlider::TicksBelow);

    // Top box setup
    clientStatusCircle = new QLabel(this);
    clientStatusCircle->setFixedSize(25, 25);
    clientStatusCircle->setStyleSheet("border-radius: 12px; background-color: green;");
    clientName = new QLabel("1001", this);
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
    clients.append({"phone1", "1001", "Online"});
    clients.append({"phone2", "1002", "Offline"});
    clients.append({"phone3", "1003", "Busy"});
    clientList = new QListWidget(this);
    populateList();
    clientBox->addWidget(clientList);

    setupCallLayouts();
    setupConferenceUI();

    // Main layout setup
    MidBox->addLayout(clientBox, 2);
    MidBox->addLayout(callLayoutsStack, 1);
    mainLayout->addLayout(topBox);
    mainLayout->addLayout(MidBox);
    mainLayout->addLayout(makeConfCallLayout);
    mainWidget->setLayout(mainLayout);

    connectSignals();
    loadThemePreference();
    applyTheme(isDarkTheme);
}

ClientWindow::~ClientWindow() {
    delete mainWidget;
    delete clientStatusCircle;
    delete clientName;
    delete themeBtn;
    delete exitBtn;
    delete logout;
    delete clientList;
    delete noCallLabel;
    delete incomingCallLabel;
    delete incomingClientLabel;
    delete ongoingClientLabel;
    delete outgoingCallLabel;
    delete outgoingClientLabel;
    delete noCallWidget;
    delete incomingCallWidget;
    delete outgoingCallWidget;
    delete ongoingCallWidget;
    delete acceptCall_btn;
    delete rejectCall_btn;
    delete leaveCall_btn;
    delete endCall_btn;
    //delete MakeConfCall_btn;
    delete conferencePanel;
    delete selectAllCheckbox;
    delete startConferenceBtn;
}

void ClientWindow::setupCallLayouts() {
    noCallLabel = new QLabel("NO active calls", this);
    noCallLayout->addWidget(noCallLabel);
    noCallWidget->setLayout(noCallLayout);

    incomingCallLabel = new QLabel("Incoming call....", this);
    incomingClientLabel = new QLabel("Client Name", this);
    acceptCall_btn = new QPushButton("Accept Call", this);
    rejectCall_btn = new QPushButton("Reject Call", this);
    incomingCallLayout->addWidget(incomingCallLabel);
    incomingCallLayout->addWidget(incomingClientLabel);
    incomingCallLayout->addWidget(acceptCall_btn);
    incomingCallLayout->addWidget(rejectCall_btn);
    incomingCallWidget->setLayout(incomingCallLayout);

    ongoingClientLabel = new QLabel("Client Name", this);
    leaveCall_btn = new QPushButton("End Call", this);
    ongoingCallLayout->addWidget(ongoingClientLabel);
    ongoingCallLayout->addWidget(leaveCall_btn);
    ongoingCallWidget->setLayout(ongoingCallLayout);

    outgoingCallLabel = new QLabel("Ringing....", this);
    outgoingClientLabel = new QLabel("Client Name", this);
    endCall_btn = new QPushButton("End Call", this);
    outgoingCallLayout->addWidget(outgoingCallLabel);
    outgoingCallLayout->addWidget(outgoingClientLabel);
    outgoingCallLayout->addWidget(endCall_btn);
    outgoingCallWidget->setLayout(outgoingCallLayout);

    //MakeConfCall_btn = new QPushButton("Make Conference Call", this);
    //makeConfCallLayout->addWidget(MakeConfCall_btn);

    callLayoutsStack = new QStackedLayout();
    callLayoutsStack->addWidget(noCallWidget);
    callLayoutsStack->addWidget(incomingCallWidget);
    callLayoutsStack->addWidget(ongoingCallWidget);
    callLayoutsStack->addWidget(outgoingCallWidget);
}

void ClientWindow::setupConferenceUI() {
    // Move conference toggle button to the main controls area
    QPushButton *conferenceToggle = new QPushButton("Conference Mode", this);
    makeConfCallLayout->addWidget(conferenceToggle); // Use the existing makeConfCallLayout

    conferencePanel = new QWidget(this);
    QVBoxLayout *confLayout = new QVBoxLayout(conferencePanel);

    // Add conference panel to the main layout to prevent shifting
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

void ClientWindow::connectSignals() {
    connect(acceptCall_btn, &QPushButton::clicked, this, &ClientWindow::onOngoingCall);
    connect(leaveCall_btn, &QPushButton::clicked, [this]() { switchToLayout(0); });
    connect(endCall_btn, &QPushButton::clicked, [this]() { switchToLayout(0); });
    connect(rejectCall_btn, &QPushButton::clicked, [this]() { switchToLayout(0); });
    connect(logout, &QPushButton::clicked, this, &ClientWindow::onLogoutBtnClicked);
    connect(exitBtn, &QPushButton::clicked, this, &ClientWindow::onExitBtnClicked);
    connect(themeBtn, &QPushButton::clicked, this, &ClientWindow::toggleTheme);
}

// Theme management methods
void ClientWindow::toggleTheme() {
    isDarkTheme = !isDarkTheme;
    applyTheme(isDarkTheme);
    saveThemePreference();
}

void ClientWindow::applyTheme(bool isDark) {
    QString styleSheet = isDark ? getDarkThemeStyleSheet() : getLightThemeStyleSheet();
    mainWidget->setStyleSheet(styleSheet);
    QString statusCircleStyle = QString("border-radius: 12px; background-color: %1;")
                                    .arg(isDark ? "#404040" : "#e0e0e0");
    clientStatusCircle->setStyleSheet(statusCircleStyle);
}

QString ClientWindow::getLightThemeStyleSheet() {
    return R"(
        QWidget {
            background-color: #E0E3DE;
            font-family: 'Segoe UI', Arial, sans-serif;
            font-size: 12px;
        }
        QPushButton {
            background-color: #4A5D45;
            color: #E0E3DE;
            border: 1px solid #2F3E2C;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #5B705A;
        }
        QPushButton[text="Call"] {
            color: #FFFFFF;
            font-weight: bold;
            background-color: #445544;
            min-width: 60px;
        }
        QPushButton[text="Call"]:hover {
            background-color: #556655;
        }
        QListWidget {
            background-color: #D3D7D1;
            border-radius: 4px;
            padding: 5px;
            border: 1px solid #A3A69F;
        }
        QLineEdit {
            padding: 8px;
            border-radius: 4px;
            border: 2px solid #A3A69F;
            background-color: #D3D7D1;
        }
        QLineEdit:focus {
            border: 2px solid #4A5D45;
        }
        QLabel {
            color: #2F3E2C;
            font-weight: 500;
        }
        QCheckBox {
            spacing: 8px;
        }
QPushButton[text="Call"] {
    color: #FFFFFF;

    background-color: #445544;
    min-width: 60px;
    min-height: 25px;
    font-size: 12px;
    text-align: center;
    padding: 2px 10px;
    qproperty-alignment: AlignCenter;
}

QPushButton[text="Call"]:hover {
    background-color: #556655;
}
    )";
}

QString ClientWindow::getDarkThemeStyleSheet() {
    return R"(
        QWidget {
            background-color: #1E2922;
            font-family: 'Segoe UI', Arial, sans-serif;
            font-size: 12px;
        }
        QPushButton {
            background-color: #3F4A3C;
            color: #D3D7D1;
            border: 1px solid #2F3E2C;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4A5D45;
        }
        QPushButton[text="Call"] {
            color: #FFFFFF;
            font-weight: bold;
            background-color: #445544;
            min-width: 60px;
        }
        QPushButton[text="Call"]:hover {
            background-color: #556655;
        }
        QListWidget {
            background-color: #2A362F;
            border-radius: 4px;
            padding: 5px;
            border: 1px solid #3F4A3C;
            color: #D3D7D1;
        }
        QLineEdit {
            padding: 8px;
            border-radius: 4px;
            border: 2px solid #3F4A3C;
            background-color: #2A362F;
            color: #D3D7D1;
        }
        QLineEdit:focus {
            border: 2px solid #4A5D45;
        }
        QLabel {
            color: #D3D7D1;
            font-weight: 500;
        }
        QCheckBox {
            spacing: 8px;
            color: #D3D7D1;
        }
QPushButton[text="Call"] {
    color: #FFFFFF;
    font-weight: bold;
    background-color: #445544;
    min-width: 60px;
    min-height: 25px;
    font-size: 12px;
    text-align: center;
    padding: 2px 10px;
    qproperty-alignment: AlignCenter;
}

QPushButton[text="Call"]:hover {
    background-color: #556655;
}
    )";
}


void ClientWindow::saveThemePreference() {
    QSettings settings("YourCompany", "VoIPClient");
    settings.setValue("darkTheme", isDarkTheme);
}

void ClientWindow::loadThemePreference() {
    QSettings settings("YourCompany", "VoIPClient");
    isDarkTheme = settings.value("darkTheme", false).toBool();
}

// Conference call methods
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

    //MakeConfCall_btn->setVisible(!isConferenceMode);
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

    // Directly start the conference call
    switchToLayout(2);
    ongoingClientLabel->setText("Conference Call: " + selectedClients.join(", "));

    // Hide conference panel after starting the call
    conferencePanel->hide();

    // Reset checkboxes
    selectAllCheckbox->setChecked(false);
    handleSelectAll(Qt::Unchecked);
}


void ClientWindow::initiateConferenceCall(const QList<QString>& participants) {
    QString participantList = participants.join(", ");
    QMessageBox::information(this, "Conference Call Started",
                             "Starting conference call with: " + participantList);
    switchToLayout(2);
    ongoingClientLabel->setText("Conference Call: " + participantList);
}

// Call management methods
void ClientWindow::switchToLayout(int index) {
    callLayoutsStack->setCurrentIndex(index);
}

void ClientWindow::onIncomingCall() {
    switchToLayout(1);
    incomingClientLabel->setText(currentClient);
}

void ClientWindow::onOngoingCall() {
    switchToLayout(2);
    ongoingClientLabel->setText(currentClient);
}

void ClientWindow::onCallBtnClicked() {
    if (!currentClient.isEmpty()) {
        outgoingClientLabel->setText(currentClient);
        switchToLayout(3);
    }
}

// Client list management
void ClientWindow::populateList() {
    for (const auto &client : clients) {
        QWidget *clientWidget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(clientWidget);

        QLabel *status = new QLabel(clientWidget);
        status->setPixmap(getStatusIcon(client.status));

        QLabel *username = new QLabel(client.username, clientWidget);
        QCheckBox *selectBox = new QCheckBox(clientWidget);
        selectBox->hide();

        QPushButton *callBtn = new QPushButton("Call", clientWidget);

        // Add connection for call button
        connect(callBtn, &QPushButton::clicked, this, [this, username]() {
            currentClient = username->text();
            onCallBtnClicked();
        });

        layout->addWidget(status);
        layout->addWidget(username);
        layout->addWidget(selectBox);
        layout->addWidget(callBtn);

        QListWidgetItem *item = new QListWidgetItem(clientList);
        item->setSizeHint(clientWidget->sizeHint());
        clientList->setItemWidget(item, clientWidget);
    }
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

QList<QString> ClientWindow::getSelectedClients() {
    QList<QString> selectedClients;
    for (int i = 0; i < clientList->count(); ++i) {
        QListWidgetItem *item = clientList->item(i);
        if (item->checkState() == Qt::Checked) {
            selectedClients.append(item->text());
        }
    }
    return selectedClients;
}

// Add this method to handle incoming calls
void ClientWindow::handleIncomingCall(const QString &caller) {
    currentClient = caller;
    incomingClientLabel->setText(caller);
    switchToLayout(1);  // Switch to incoming call layout
}

void ClientWindow::onCallAccepted() {
    ongoingClientLabel->setText(currentClient);
    switchToLayout(2);  // Switch to ongoing call layout
}

void ClientWindow::onCallRejected() {
    switchToLayout(0);  // Switch back to no call layout
    currentClient.clear();
}


