QT       += core gui
QT       += statemachine

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialog_about.cpp \
    fw_upgrade.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    dialog_about.h \
    fw_upgrade.h \
    mainwindow.h

FORMS += \
    dialog_about.ui \
    mainwindow.ui

TRANSLATIONS += \
    NTX_Firmware_Upgrade_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

RESOURCES += \
    Resource.qrc

DISTFILES += \
    Firmware.ini


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /usr/local/bin/
!isEmpty(target.path): INSTALLS += target


unix|win32: LIBS += -L$$PWD/../Library_3rdParty/Shell_Interactive/lib64/ -lShell_Interactive

INCLUDEPATH += $$PWD/../Library_3rdParty/Shell_Interactive/include
DEPENDPATH += $$PWD/../Library_3rdParty/Shell_Interactive/include

unix|win32: LIBS += -L$$PWD/../Library_3rdParty/File_Searcher/lib64/ -lFile_Searcher

INCLUDEPATH += $$PWD/../Library_3rdParty/File_Searcher/include
DEPENDPATH += $$PWD/../Library_3rdParty/File_Searcher/include

unix|win32: LIBS += -L$$PWD/../Library_3rdParty/File_Copy/lib64/ -lFile_Copy

INCLUDEPATH += $$PWD/../Library_3rdParty/File_Copy/include
DEPENDPATH += $$PWD/../Library_3rdParty/File_Copy/include
