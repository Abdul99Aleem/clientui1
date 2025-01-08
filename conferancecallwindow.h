    //conferancecallwindow.h
    #ifndef CONFERANCECALLWINDOW_H
    #define CONFERANCECALLWINDOW_H

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
    #include <QCheckBox>
    #include <QInputDialog>
    #include <QFile>
    #include <QRegularExpression>
    #include <QRegularExpressionMatch>
    #include <QProcess>
    #include "mainwindow.h"
    #include <QSlider>

    class ConferanceCallWindow : public QMainWindow
    {
        Q_OBJECT
    public:
        explicit ConferanceCallWindow(QWidget *parent = nullptr);
        //  ConferanceCallWindow();
        void createClientCheckboxes(const QStringList &clients);
        void onStartCall();
        void onStartCallBtnClicked();

        QList<QCheckBox*> checkboxes;
        QPushButton *StartCall_btn;

    private:
        QVBoxLayout *ConfClientListLayout;
        QHBoxLayout *MakeCallLayout;
        QVBoxLayout *ConfWinMainLayout;

    signals:
        void startCallClicked();
    };

    #endif // CONFERANCECALLWINDOW_H
