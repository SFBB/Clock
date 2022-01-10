#include(../rasterwindow/rasterwindow.pri)

# work-around for QTBUG-13496
CONFIG += no_batch

SOURCES += \
    main.cpp

INSTALLS += target

QT += widgets
      gui
      core

LIBS += -L/usr/X11/lib -lX11
