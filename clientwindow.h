//clientwindow.h
#ifndef CLIENTLIST
#define CLIENTLIST

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
#include "clientdata.h"
#include <QSlider>
#include "conferancecallwindow.h"
#include <messagewindow.h>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QAudioDevice>
#include <QMediaDevices>

class ConferanceCallWindow;
class MainWindow;

class ClientWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    explicit ClientWindow(MainWindow *mainWindow, QWidget *parent = nullptr);
    ClientWindow();
    ~ClientWindow();

    void on_MakeConfCall_btn_clicked();
    void populateList();
    void switchToLayout(int);
    void endCallClicked();
    void onOngoingCall();
    void onIncomingCall();
    void onCallBtnClicked();
    void onLogoutBtnClicked();
    void onExitBtnClicked();
    QList<QString> getSelectedClients();

public slots:
    void handleIncomingCall(const QString &caller);
    void onCallAccepted();
    void onCallRejected();
    void removeMessageWindow(const QString &username);
    void showHomeScreen();
    void showMessageScreen(const QString &username);
    void handleServerUpdate(const QByteArray &data);

private:
    void setupCallLayouts();
    void connectSignals();
    void loadThemePreference();
    void saveThemePreference();
    void setupUI();
    void setupConnections();
    QString getDarkThemeStyleSheet();
    QString getLightThemeStyleSheet();
    void toggleTheme();
    void applyTheme(bool isDark);
    void setupConferenceUI();
    void handleSelectAll(Qt::CheckState state);
    void startConference();
    void initiateConferenceCall(const QList<QString>& participants);
    void toggleConferenceMode();
    QPixmap getStatusIcon(const QString &status);

    // Message window related
    QMap<QString, MessageWindow*> messageWindows;
    void openMessageWindow(const QString &username);
    QStackedWidget *mainStack;
    QWidget *homeScreen;
    QWidget *rightPanel;

    bool isDarkTheme = false;
    QWidget *mainWidget;
    MainWindow *mainWindow;

    // Layouts
    QHBoxLayout *topBox;
    QHBoxLayout *clientBox;
    QHBoxLayout *makeConfCallLayout;
    QHBoxLayout *MidBox;
    QVBoxLayout *callStatusLayout;
    QVBoxLayout *incomingCallLayout;
    QVBoxLayout *ongoingCallLayout;
    QVBoxLayout *outgoingCallLayout;
    QVBoxLayout *noCallLayout;
    QVBoxLayout *mainLayout;
    QStackedLayout *callLayoutsStack;

    // Conference related
    QWidget *conferencePanel;
    QCheckBox *selectAllCheckbox;
    QPushButton *startConferenceBtn;

    // Labels and widgets
    QLabel *clientStatusCircle;
    QLabel *clientName;
    QLabel *noCallLabel;
    QLabel *incomingCallLabel;
    QLabel *incomingClientLabel;
    QLabel *ongoingClientLabel;
    QLabel *outgoingCallLabel;
    QLabel *outgoingClientLabel;
    QWidget *noCallWidget;
    QWidget *incomingCallWidget;
    QWidget *outgoingCallWidget;
    QWidget *ongoingCallWidget;

    // Buttons
    QPushButton *themeBtn;
    QPushButton *exitBtn;
    QPushButton *logout;
    QPushButton *acceptCall_btn;
    QPushButton *rejectCall_btn;
    QPushButton *leaveCall_btn;
    QPushButton *endCall_btn;

    // Other members
    QListWidget *clientList;
    QList<ClientData> clients;
    QString currentClient;

    QAudioDevice audioDevice;
    QSlider *volumeSlider;
    void handleVolumeChange(int value);
};

#endif // CLIENTLIST
