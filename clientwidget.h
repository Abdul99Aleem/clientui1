//clientwidget.h
#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class ClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClientWidget(const QString &username, const QString &dialplan, const QString &password, const QString &status, QWidget *parent = nullptr);
    ~ClientWidget();

signals:
    void callButtonClicked(const QString &username);
    void messageButtonClicked(const QString &username);

private slots:
    void onCallButtonClicked();

private:
    QString username;
    QString dialplan;
    QString password;
    QString status;
    void updateStatusCircle(const QString &status);
    QPushButton *messageButton;
    void onMessageButtonClicked();
    QLabel *statusCircle;
    QLabel *usernameLabel;
    QPushButton *callButton;
    QString clientUsername;
};

#endif // CLIENTWIDGET_H
