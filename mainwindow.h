//mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QSettings>

#include <QMainWindow>
#include <QDialog>
#include <QWebSocket>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <clientwindow.h>
#include "clientdata.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString getUsername() const;
    QString getPassword() const;
    QString getIPaddr() const;
    void validateInputs();
    void clearCredentials();
    QCheckBox *rememberMe;
    void on_SignIn_btn_clicked();
    void saveCredentials(const QString &username, const QString &password, const QString &IPaddr);
    bool loadCredentials(QString &username, QString &password, QString &IPaddr);

private:
    Ui::MainWindow *ui;

    QLineEdit *username;
    QLineEdit *password;
    QLineEdit *IPaddr;
    QPushButton *SignInButton;

    QLabel *errorLabel;
    QString savedUsername;
    QString savedPassword;
    QString savedIPaddr;

    bool isValidIPAddress(const QString &ip);

    QWebSocket *webSocket;
    QListWidget *clientList;
    void populateClientList(const QList<ClientData> &clients);

private slots:
    void onWebSocketConnected();
    void onWebSocketMessageReceived(const QString &message);

};
#endif // MAINWINDOW_H
