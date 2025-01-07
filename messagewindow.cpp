//messagewindow.cpp
#include "messagewindow.h"
#include <QDateTime>
#include <QFileDialog>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMenu>

MessageWindow::MessageWindow(const QString &username, bool isDarkTheme, QWidget *parent)
    : QWidget(parent), username(username), isDarkTheme(isDarkTheme)
{
    setupUI();
    connectSignals();
    loadChatHistory();
    applyTheme(isDarkTheme);
    createEmojiPanel();
    setupContextMenu();

    // Enable drag and drop
    setAcceptDrops(true);
    chatDisplay->setAcceptDrops(true);
    chatDisplay->installEventFilter(this);
}

void MessageWindow::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Header setup
    headerLayout = new QHBoxLayout();
    backButton = new QPushButton("←", this);
    backButton->setFixedSize(30, 30);
    usernameLabel = new QLabel(username, this);
    usernameLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(backButton);
    headerLayout->addWidget(usernameLabel, 1);
    headerLayout->addStretch();

    // Chat display setup
    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);
    chatDisplay->setMinimumHeight(300);

    // Input area setup
    inputLayout = new QHBoxLayout();
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("Type a message...");

    emojiButton = new QPushButton("😊", this);
    attachButton = new QPushButton("📎", this);
    sendButton = new QPushButton("Send", this);

    inputLayout->addWidget(emojiButton);
    inputLayout->addWidget(attachButton);
    inputLayout->addWidget(messageInput, 1);
    inputLayout->addWidget(sendButton);

    // Action buttons setup
    QHBoxLayout *actionLayout = new QHBoxLayout();
    clearButton = new QPushButton("Clear Chat", this);
    exportButton = new QPushButton("Export Chat", this);

    actionLayout->addWidget(clearButton);
    actionLayout->addWidget(exportButton);

    // Add all layouts to main layout
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(chatDisplay, 1);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(actionLayout);

    // Set size policy
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(400, 500);
}
void MessageWindow::connectSignals()
{
    connect(backButton, &QPushButton::clicked, this, &MessageWindow::backButtonClicked);
    connect(sendButton, &QPushButton::clicked, this, &MessageWindow::sendMessage);
    connect(messageInput, &QLineEdit::returnPressed, this, &MessageWindow::handleReturnPressed);
    connect(clearButton, &QPushButton::clicked, this, &MessageWindow::clearChat);
    connect(exportButton, &QPushButton::clicked, this, &MessageWindow::exportChat);
    connect(attachButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, "Attach File");
        if (!filePath.isEmpty()) {
            handleFileDrop(filePath);
        }
    });
}

void MessageWindow::sendMessage()
{
    QString message = messageInput->text().trimmed();
    if (message.isEmpty()) return;

    if (message.length() > MAX_MESSAGE_LENGTH) {
        QMessageBox::warning(this, "Message Too Long",
                             "Message exceeds maximum length of " + QString::number(MAX_MESSAGE_LENGTH) + " characters.");
        return;
    }

    addMessage("Me", message);
    emit messageSent(username, message);
    messageInput->clear();
}

void MessageWindow::addMessage(const QString &sender, const QString &message)
{
    QString formattedMessage = formatMessage(sender, message);
    chatDisplay->append(formattedMessage);

    messageHistory.append(qMakePair(sender, message));
    if (messageHistory.size() > MAX_HISTORY_SIZE) {
        messageHistory.removeFirst();
    }

    scrollToBottom();
    saveChatHistory();
}

QString MessageWindow::formatMessage(const QString &sender, const QString &message)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeStr = currentTime.toString(DATE_FORMAT);

    QString formattedMsg = QString("<p><b>[%1] %2:</b> %3</p>")
                               .arg(timeStr)
                               .arg(sender)
                               .arg(message);

    return formattedMsg;
}

void MessageWindow::scrollToBottom()
{
    QScrollBar *scrollBar = chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MessageWindow::handleReturnPressed()
{
    sendMessage();
}
void MessageWindow::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    applyTheme(isDark);
}

void MessageWindow::applyTheme(bool isDark)
{
    setStyleSheet(isDark ? getDarkThemeStyleSheet() : getLightThemeStyleSheet());
}

QString MessageWindow::getDarkThemeStyleSheet()
{
    return R"(
        QWidget {
            background-color: #1E2922;
            color: #E0E3DE;
        }
        QTextEdit {
            background-color: #2A362F;
            border: 1px solid #3F4A3C;
            border-radius: 4px;
        }
        QLineEdit {
            background-color: #2A362F;
            border: 1px solid #3F4A3C;
            border-radius: 4px;
            padding: 5px;
            color: #E0E3DE;
        }
        QPushButton {
            background-color: #3F4A3C;
            border: 1px solid #2F3E2C;
            border-radius: 4px;
            padding: 5px 10px;
            color: #FFFFFF;
        }
        QPushButton:hover {
            background-color: #4A5D45;
        }
        QLabel {
            color: #E0E3DE;
        }
    )";
}

QString MessageWindow::getLightThemeStyleSheet()
{
    return R"(
        QWidget {
            background-color: #E0E3DE;
            color: #2F3E2C;
        }
        QTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #A3A69F;
            border-radius: 4px;
        }
        QLineEdit {
            background-color: #FFFFFF;
            border: 1px solid #A3A69F;
            border-radius: 4px;
            padding: 5px;
            color: #2F3E2C;
        }
        QPushButton {
            background-color: #4A5D45;
            border: 1px solid #2F3E2C;
            border-radius: 4px;
            padding: 5px 10px;
            color: #FFFFFF;
        }
        QPushButton:hover {
            background-color: #5B705A;
        }
        QLabel {
            color: #2F3E2C;
        }
    )";
}

void MessageWindow::handleFileDrop(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    QString message = QString("📎 Attached file: %1").arg(fileName);
    addMessage("Me", message);
}

bool MessageWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == chatDisplay) {
        if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent*>(event);
            if (dragEvent->mimeData()->hasUrls()) {
                dragEvent->acceptProposedAction();
                return true;
            }
        }
        else if (event->type() == QEvent::Drop) {
            QDropEvent *dropEvent = static_cast<QDropEvent*>(event);
            const QMimeData* mimeData = dropEvent->mimeData();
            if (mimeData->hasUrls()) {
                QString filePath = mimeData->urls().first().toLocalFile();
                handleFileDrop(filePath);
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
void MessageWindow::loadChatHistory()
{
    QString filePath = getChatFilePath();
    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        chatDisplay->clear();

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split("|||");
            if (parts.size() == 2) {
                messageHistory.append(qMakePair(parts[0], parts[1]));
                chatDisplay->append(formatMessage(parts[0], parts[1]));
            }
        }
        file.close();
        scrollToBottom();
    }
}

void MessageWindow::saveChatHistory()
{
    QString filePath = getChatFilePath();
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const auto &message : messageHistory) {
            out << message.first << "|||" << message.second << "\n";
        }
        file.close();
    }
}

QString MessageWindow::getChatFilePath()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    return dataPath + "/" + username + "_chat.txt";
}

void MessageWindow::clearChat()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Clear Chat",
        "Are you sure you want to clear the chat history?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        chatDisplay->clear();
        messageHistory.clear();
        saveChatHistory();
    }
}

void MessageWindow::exportChat()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Chat History",
        QDir::homePath() + "/" + username + "_chat_export.txt",
        "Text Files (*.txt)"
        );

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << chatDisplay->toPlainText();
            file.close();

            QMessageBox::information(
                this,
                "Export Successful",
                "Chat history has been exported successfully."
                );
        }
    }
}

void MessageWindow::closeEvent(QCloseEvent *event)
{
    saveChatHistory();
    emit closed();
    event->accept();
}

MessageWindow::~MessageWindow()
{
    saveChatHistory();
}
void MessageWindow::createEmojiPanel()
{
    emojiPanel = new QWidget(nullptr);
    emojiPanel->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

    QGridLayout *emojiLayout = new QGridLayout(emojiPanel);
    emojiLayout->setSpacing(5);  // Increased spacing between emojis
    emojiLayout->setContentsMargins(8, 8, 8, 8);  // Increased margins

    // Initialize emoji map
    emojiMap = {
        {"smile", "😊"}, {"laugh", "😄"}, {"wink", "😉"},
        {"heart", "❤️"}, {"thumbsup", "👍"}, {"clap", "👏"},
        {"think", "🤔"}, {"cool", "😎"}, {"party", "🎉"},
        {"sad", "😢"}, {"angry", "😠"}, {"love", "😍"}
    };

    int row = 0, col = 0;
    for (auto it = emojiMap.begin(); it != emojiMap.end(); ++it) {
        QPushButton *emojiBtn = new QPushButton(it.value());
        emojiBtn->setFixedSize(40, 40);  // Increased button size
        emojiBtn->setStyleSheet("QPushButton { border: none; background: transparent; font-size: 20px; }"); // Increased font size

        connect(emojiBtn, &QPushButton::clicked, this, [this, emoji = it.value()]() {
            handleEmojiInsert(emoji);
        });

        emojiLayout->addWidget(emojiBtn, row, col);
        col++;
        if (col > 3) {
            col = 0;
            row++;
        }
    }

    connect(emojiButton, &QPushButton::clicked, this, &MessageWindow::positionEmojiPanel);
}



void MessageWindow::setupContextMenu()
{
    chatDisplay->setContextMenuPolicy(Qt::CustomContextMenu);
    contextMenu = new QMenu(this);

    QAction *copyAction = new QAction("Copy", this);
    QAction *selectAllAction = new QAction("Select All", this);

    contextMenu->addAction(copyAction);
    contextMenu->addAction(selectAllAction);

    connect(copyAction, &QAction::triggered, chatDisplay, &QTextEdit::copy);
    connect(selectAllAction, &QAction::triggered, chatDisplay, &QTextEdit::selectAll);

    connect(chatDisplay, &QTextEdit::customContextMenuRequested,
            this, [this](const QPoint &pos) {
                contextMenu->exec(chatDisplay->mapToGlobal(pos));
            });
}
void MessageWindow::positionEmojiPanel()
{
    if (emojiPanel->isVisible()) {
        emojiPanel->hide();
    } else {
        QPoint globalPos = emojiButton->mapToGlobal(QPoint(0, 0));
        emojiPanel->move(globalPos.x(), globalPos.y() - emojiPanel->height());
        emojiPanel->show();
    }
}
void MessageWindow::handleEmojiInsert(const QString &emoji)
{
    messageInput->insert(emoji);
    emojiPanel->hide();
}

void MessageWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (emojiPanel->isVisible()) {
        QPoint pos = emojiButton->mapToGlobal(QPoint(0, -emojiPanel->height()));
        emojiPanel->move(pos);
    }
}
