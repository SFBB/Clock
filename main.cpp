/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>

//#include "rasterwindow.h"
 #include <QApplication>
#include <iostream>
#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QResource>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/X.h>



class AnalogClockWindow : public QWidget
{
public:
    AnalogClockWindow(QWidget* parent);
    ~AnalogClockWindow();

protected:
    void timerEvent(QTimerEvent *) override;
    void render(QPainter *p);
    void render();
    void updateState();
    void paintEvent(QPaintEvent* event) override;
    void createSystemTray();

private slots:
    void update_pos();

private:
    float ratio = 0.2, displayHeight = 100, displayWidth = 100, screenWidth = 100, screenHeight = 100, windowWidth = 100, windowHeight = 100;
    int m_timerId;
    QSystemTrayIcon *trayIcon = nullptr;
    QAction *hideAction;
    QAction *showAction;
    QAction *smartHideAction;
    QAction *quitAction;
    QMenu *trayIconMenu;
    Display* primaryDisplay;

    bool forceHide = false, forceShow = false;
    void hideWindow();
    void showWindow();
    void smartHideWindow();
};



AnalogClockWindow::AnalogClockWindow(QWidget* parent)
    : QWidget(parent)
{
    createSystemTray();
    if(trayIcon)
        trayIcon->show();

    setWindowTitle("Analog Clock");
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    // setWindowFlag(Qt::WindowDoesNotAcceptFocus);
    setWindowFlag(Qt::Tool);
    setWindowFlag(Qt::Dialog);
    setWindowFlag(Qt::WindowTransparentForInput);
    setWindowFlag(Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    // setStyleSheet("");
    primaryDisplay = XOpenDisplay(NULL);
    int screen_num = DefaultScreen(primaryDisplay);
    displayWidth = DisplayWidth(primaryDisplay, screen_num), displayHeight = DisplayHeight(primaryDisplay, screen_num);
    screenWidth = QApplication::primaryScreen()->size().width(), screenHeight = QApplication::primaryScreen()->size().height();
    ratio = 0.2;
    windowWidth = qMin(screenWidth*ratio, screenHeight*ratio), windowHeight = windowWidth;
    // resize(width, height);
    std::cout << "Screen Height: " << screenHeight << ", Screen Width: " << screenWidth << std::endl;
    std::cout << "Ratio: " << ratio << ", Height: " << windowHeight << ", width: " << windowWidth << std::endl;
    setGeometry(QRect(screenWidth-windowWidth, 0, windowWidth, windowHeight));

    m_timerId = startTimer(1000);
//    timer->start(50); // update interval in milliseconds
}

AnalogClockWindow::~AnalogClockWindow()
{
    std::cout << "Quited!" << std::endl;
}

void AnalogClockWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId)
    {
        // std::cout << "clock" << std::endl;
        updateState();
        update();
        // setWindowState(Qt::WindowActive);
    }
}

void AnalogClockWindow::updateState()
{
    Window w;
    int revert_to;
    XGetInputFocus(primaryDisplay, &w, &revert_to);
    if(w == None)
    {
        std::cout << "Error!" << std::endl;
    }
    XWindowAttributes window_attributes;
    Status s = XGetWindowAttributes(primaryDisplay, w, &window_attributes);
    if (window_attributes.height == 1 && window_attributes.width == 1)
    {
        Window parent, root;
        Window* children;
        unsigned int num_children;
        if(XQueryTree(primaryDisplay, w, &root, &parent, &children, &num_children))
        {
            XGetWindowAttributes(primaryDisplay, parent, &window_attributes);
        }
    }
//    std::cout<< window_attributes.height << ", " << window_attributes.width << std::endl;
    if(forceShow || ((!forceHide) && (window_attributes.height == displayHeight && window_attributes.width == displayWidth)))
    {
        showNormal();
    }
    else
    {
        hide();
    }
}

void AnalogClockWindow::update_pos()
{
//    screenWidth = QApplication::primaryScreen()->size().width(), screenHeight = QApplication::primaryScreen()->size().height();
//    ratio = 0.2;
//    windowWidth = qMin(screenWidth*ratio, screenHeight*ratio), windowHeight = windowWidth;
    setGeometry(QRect(screenWidth-windowWidth, 0, windowWidth, screenHeight));
}

void AnalogClockWindow::createSystemTray()
{

    QResource::registerResource("/home/tomtony/Documents/Clock/Clock/qr.rcc");
    hideAction = new QAction(tr("Hide"), this);
    connect(hideAction, &QAction::triggered, this, &AnalogClockWindow::hideWindow);
    showAction = new QAction(tr("&Show"), this);
    connect(showAction, &QAction::triggered, this, &AnalogClockWindow::showWindow);
    smartHideAction = new QAction(tr("&Smart Hide"), this);
    connect(smartHideAction, &QAction::triggered, this, &AnalogClockWindow::smartHideWindow);
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(hideAction);
    trayIconMenu->addAction(showAction);
    trayIconMenu->addAction(smartHideAction);
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/Icons/systemTray.png"));

    trayIcon->setVisible(true);
    hideAction->setVisible(true);
    showAction->setVisible(true);
    smartHideAction->setVisible(true);
    quitAction->setVisible(true);
}

void AnalogClockWindow::hideWindow()
{
    forceHide = true;
    forceShow = false;
    updateState();
}

void AnalogClockWindow::showWindow()
{
    forceHide = false;
    forceShow = true;
    updateState();
}

void AnalogClockWindow::smartHideWindow()
{
    forceHide = false;
    forceShow = false;
    updateState();
}

void AnalogClockWindow::paintEvent(QPaintEvent* event)
{
    render();
}

void AnalogClockWindow::render()
{
    // QRect rect(0, 0, width(), height());
    // m_backingStore->beginPaint(rect);

    QPaintDevice *device = this;
    QPainter painter(device);

    // painter.fillRect(0, 0, width(), height(), QGradient::NightFade);
    render(&painter);
    painter.end();
    // renderLater();
}

void AnalogClockWindow::render(QPainter *p)
{
    static const QPoint hourHand[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -40)
    };
    static const QPoint hourHandPlus[3] = {
        QPoint(3, 8),
        QPoint(-3, 8),
        QPoint(0, -40)
    };
    static const QPoint minuteHand[3] = {
        QPoint(7, 8),
        QPoint(-7, 8),
        QPoint(0, -70)
    };
    static const QPoint minuteHandPlus[3] = {
        QPoint(3, 8),
        QPoint(-3, 8),
        QPoint(0, -70)
    };

    QColor hourColor(127, 0, 127);
    QColor hourColorPlus(255, 255, 255);
    QColor minuteColor(0, 127, 127, 191);
    QColor minuteColorPlus(255, 255, 255);

    p->setRenderHint(QPainter::Antialiasing);

    p->translate(width() / 2, height() / 2);

    int side = qMin(width(), height());
    p->scale(side / 200.0, side / 200.0);

    p->setPen(Qt::NoPen);
    p->setBrush(hourColor);

    QTime time = QTime::currentTime();

    p->save();
    p->rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
    p->drawConvexPolygon(hourHand, 3);
    p->setBrush(hourColorPlus);
    p->drawConvexPolygon(hourHandPlus, 3);
    p->restore();

    p->setPen(hourColor);

    for (int i = 0; i < 12; ++i) {
        p->drawLine(88, 0, 96, 0);
        p->rotate(30.0);
    }

    p->setPen(Qt::NoPen);
    p->setBrush(minuteColor);

    p->save();
    p->rotate(6.0 * (time.minute() + time.second() / 60.0));
    p->drawConvexPolygon(minuteHand, 3);
    p->setBrush(minuteColorPlus);
    p->drawConvexPolygon(minuteHandPlus, 3);
    p->restore();

    p->setPen(minuteColor);

    for (int j = 0; j < 60; ++j) {
        if ((j % 5) != 0)
            p->drawLine(92, 0, 96, 0);
        p->rotate(6.0);
    }
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    AnalogClockWindow clock(app.activeWindow());
    clock.show();

    return app.exec();
}
