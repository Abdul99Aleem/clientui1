QT       += core gui multimedia websockets \
    quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    clientwindow.cpp \
    clientwidget.cpp \
    conferancecallwindow.cpp \
    messagewindow.cpp

HEADERS += \
    clientdata.h \
    mainwindow.h \
    clientwindow.h \
    clientwidget.h \
    conferancecallwindow.h \
    messagewindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
