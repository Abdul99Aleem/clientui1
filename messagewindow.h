//messagewindow.h
#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QDateTime>
#include <QScrollBar>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QCloseEvent>
#include <QScopedPointer>
#include <QMutex>

class MessageWindow : public QWidget {
    Q_OBJECT

public:
    explicit MessageWindow(const QString &username, bool isDarkTheme, QWidget *parent = nullptr);
    ~MessageWindow();

    void updateTheme(bool isDarkTheme);
    void addMessage(const QString &sender, const QString &message);
    void loadChatHistory();
    void saveChatHistory();

signals:
    void backButtonClicked();
    void closed();
    void messageSent(const QString &recipient, const QString &message);
    void messageReceived(const QString &sender, const QString &message);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void sendMessage();
    void handleReturnPressed();
    void clearChat();
    void exportChat();
    void scrollToBottom();
    void handleEmojiInsert(const QString &emoji);

private:
    void setupUI();
    void connectSignals();
    void applyTheme(bool isDark);
    QString formatMessage(const QString &sender, const QString &message);
    QString getLightThemeStyleSheet();
    QString getDarkThemeStyleSheet();
    void createEmojiPanel();
    void setupContextMenu();
    QString getChatFilePath();
    void handleFileDrop(const QString &filePath);
    void positionEmojiPanel();

    // Core properties
    QString username;
    bool isDarkTheme;
    QString chatFilePath;

    // UI Elements
    QVBoxLayout *mainLayout;
    QHBoxLayout *headerLayout;
    QHBoxLayout *inputLayout;

    QPushButton *backButton;
    QLabel *usernameLabel;
    QTextEdit *chatDisplay;
    QLineEdit *messageInput;
    QPushButton *sendButton;
    QPushButton *emojiButton;
    QPushButton *attachButton;
    QPushButton *clearButton;
    QPushButton *exportButton;
    QMutex chatMutex;

    QScopedPointer<QWidget> emojiPanel;
    QScopedPointer<QMenu> contextMenu;

    // Message History
    QList<QPair<QString, QString>> messageHistory;
    QMap<QString, QString> emojiMap;

    // Constants
    static const int MAX_MESSAGE_LENGTH = 1000;
    static const int MAX_HISTORY_SIZE = 1000;
    static const int MAX_EMOJI_COUNT = 12;
    static const char* DATE_FORMAT;
};

#endif // MESSAGEWINDOW_H
