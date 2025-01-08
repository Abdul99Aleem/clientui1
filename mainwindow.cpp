//mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clientwindow.h"
#include "clientwidget.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QLabel *usernameLabel = new QLabel("Username:", this);
    username = new QLineEdit(this);

    QLabel *passwordLabel = new QLabel("Password:", this);
    password = new QLineEdit(this);
    password->setEchoMode(QLineEdit::Password);

    QLabel *IPaddrLabel = new QLabel("IP address:", this);
    IPaddr = new QLineEdit(this);

    QLabel *RememberMeLabel = new QLabel("Remember Me", this);
    rememberMe = new QCheckBox(this);

    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("color: red;");
    errorLabel->setVisible(false);

    if (loadCredentials(savedUsername, savedPassword, savedIPaddr)) {
        username->setText(savedUsername);
        password->setText(savedPassword);
        IPaddr->setText(savedIPaddr);
        rememberMe->setChecked(true);
    }

    // Initialize WebSocket
    webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    connect(webSocket, &QWebSocket::connected, this, &MainWindow::onWebSocketConnected);
    connect(webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onWebSocketMessageReceived);
    webSocket->open(QUrl("ws://localhost:12345")); // Replace with your server URL
    clientList = new QListWidget(this);
    setCentralWidget(clientList);

    SignInButton = new QPushButton("Sign in", this);
    connect(SignInButton, &QPushButton::clicked, this, &MainWindow::validateInputs);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    connect(SignInButton, &QPushButton::clicked, this, &MainWindow::on_SignIn_btn_clicked);
    buttonLayout->addWidget(SignInButton);

    QHBoxLayout *RememberMeLayout = new QHBoxLayout();
    RememberMeLayout->addWidget(RememberMeLabel);
    RememberMeLayout->addWidget(rememberMe);

    QHBoxLayout *usernameLayout = new QHBoxLayout();
    usernameLayout->addWidget(usernameLabel);
    usernameLayout->addWidget(username);

    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(password);

    QHBoxLayout *IPaddrLayout = new QHBoxLayout();
    IPaddrLayout->addWidget(IPaddrLabel);
    IPaddrLayout->addWidget(IPaddr);

    QHBoxLayout *errorLayout = new QHBoxLayout();
    errorLayout->addWidget(errorLabel);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(usernameLayout);
    mainLayout->addLayout(passwordLayout);
    mainLayout->addLayout(IPaddrLayout);
    mainLayout->addLayout(RememberMeLayout);
    mainLayout->addLayout(errorLayout);
    mainLayout->addLayout(buttonLayout);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

bool MainWindow::isValidIPAddress(const QString &ip) {
    QStringList octets = ip.split(".");

    if (octets.size() != 4) {
        return false;
    }

    for (const QString &octet : octets) {
        bool ok;
        int value = octet.toInt(&ok);

        if (!ok || value < 0 || value > 255) {
            return false;
        }

        if (octet.length() > 1 && octet.startsWith('0')) {
            return false;
        }
    }

    return true;
}

void MainWindow::validateInputs() {
    errorLabel->clear();
    errorLabel->setVisible(false);

    if (username->text().isEmpty()) {
        errorLabel->setText("Username cannot be empty");
        errorLabel->setVisible(true);
        return;
    }

    if (password->text().isEmpty()) {
        errorLabel->setText("Password cannot be empty");
        errorLabel->setVisible(true);
        return;
    }

    QString ipAddress = IPaddr->text();
    if (ipAddress.isEmpty()) {
        errorLabel->setText("IP address cannot be empty");
        errorLabel->setVisible(true);
        return;
    }

    if (!isValidIPAddress(ipAddress)) {
        errorLabel->setText("Invalid IP address format. Use format: xxx.xxx.xxx.xxx");
        errorLabel->setVisible(true);
        return;
    }
}

void MainWindow::on_SignIn_btn_clicked()
{
    validateInputs();
    if (errorLabel->isVisible()) return;

    if (rememberMe->isChecked()) {
        saveCredentials(username->text(), password->text(), IPaddr->text());
    } else {
        clearCredentials();
    }

    ClientWindow *window = new ClientWindow(this);
    window->show();
    this->hide();
}

void MainWindow::saveCredentials(const QString &username, const QString &password, const QString &IPaddr)
{
    QSettings settings("YourCompany", "VoIPClient");
    settings.setValue("username", username);
    settings.setValue("password", password);
    settings.setValue("ipaddress", IPaddr);
    settings.setValue("rememberMe", true);
}

bool MainWindow::loadCredentials(QString &username, QString &password, QString &IPaddr)
{
    QSettings settings("YourCompany", "VoIPClient");
    if (settings.value("rememberMe", false).toBool()) {
        username = settings.value("username").toString();
        password = settings.value("password").toString();
        IPaddr = settings.value("ipaddress").toString();
        return true;
    }
    return false;
}

void MainWindow::clearCredentials()
{
    QSettings settings("YourCompany", "VoIPClient");
    settings.clear();
    if (!rememberMe->isChecked()) {
        username->clear();
        password->clear();
        IPaddr->clear();
    }
}

QString MainWindow::getUsername() const
{
    return username->text();
}

QString MainWindow::getPassword() const
{
    return password->text();
}

QString MainWindow::getIPaddr() const
{
    return IPaddr->text();
}

void MainWindow::onWebSocketConnected() {
    qDebug() << "Connected to WebSocket server";
}

void MainWindow::onWebSocketMessageReceived(const QString &message) {
    qDebug() << "Message received:" << message;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isArray()) {
        QJsonArray clientArray = doc.array();
        QList<ClientData> clients;
        for (const QJsonValue &value : clientArray) {
            clients.append(ClientData{value.toString(), "", "", "OFFLINE"});
        }
        populateClientList(clients); // Update the UI with the new client list
    }
}

void MainWindow::populateClientList(const QList<ClientData> &clients) {
    clientList->clear();
    for (const ClientData &client : clients) {
        ClientWidget *widget = new ClientWidget(
            client.username,
            client.dialplan,
            client.password,
            client.status,
            this
            );
        QListWidgetItem *item = new QListWidgetItem(clientList);
        item->setSizeHint(widget->sizeHint());
        clientList->addItem(item);
        clientList->setItemWidget(item, widget);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete username;
    delete password;
    delete IPaddr;
    delete SignInButton;
    delete errorLabel;
    delete rememberMe;
}
