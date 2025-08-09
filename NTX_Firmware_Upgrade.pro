QT       += core gui
QT       += statemachine

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialog_about.cpp \
    filecopyer.cpp \
    fw_upgrade.cpp \
    main.cpp \
    mainwindow.cpp
    # flash_state.cpp \


HEADERS += \
    dialog_about.h \
    filecopyer.h \
    fw_upgrade.h \
    mainwindow.h
# flash_state.h \


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
else: unix:!android: target.path = /tmp/disk/home/root
!isEmpty(target.path): INSTALLS += target


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../Library/ -lShell_Interactive
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../Library/ -lShell_Interactive
else:unix: LIBS += -L$$OUT_PWD/../../Library/ -lShell_Interactive

INCLUDEPATH += $$PWD/../../Library
DEPENDPATH += $$PWD/../../Library

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../Library/ -lFile_Searcher
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../Library/ -lFile_Searcher
else:unix: LIBS += -L$$OUT_PWD/../../Library/ -lFile_Searcher

INCLUDEPATH += $$PWD/../../Library
DEPENDPATH += $$PWD/../../Library
