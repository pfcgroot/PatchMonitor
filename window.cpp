/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
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

#include "window.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QtGui>

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMessageBox>

//! [0]
Window::Window()
{
    patchState = patchStateUnknown;

//    createIconGroupBox();
//    createMessageGroupBox();

//    iconLabel->setMinimumWidth(durationLabel->sizeHint().width());

    createActions();
    createTrayIcon();

//    connect(showMessageButton, SIGNAL(clicked()), this, SLOT(showMessage()));
//    connect(showIconCheckBox, SIGNAL(toggled(bool)), trayIcon, SLOT(setVisible(bool)));
//    connect(iconComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setIcon(int)));
    connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    bodyLabel = new QLabel(tr("Body:"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
//    mainLayout->addWidget(iconGroupBox);
//    mainLayout->addWidget(messageGroupBox);
    mainLayout->addWidget(bodyLabel, 0, 0);
    setLayout(mainLayout);


//    iconComboBox->setCurrentIndex(1);
    trayIcon->show();

    setWindowTitle(tr("Patch Status"));
    resize(200, 150);

    QDir dir("G:/Site/Patches");
    if (!dir.exists())
        dir = "G:/Patch"; // on simulator
    if (dir.exists())
    {
        // monitor the patch directory...
        QStringList dirs(dir.absolutePath());
        fsWatcher = new QFileSystemWatcher(dirs,this);
        connect(fsWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(folderChanged(QString)));
        folderChanged(dir.path());
    }
    else
    {
        // A regular MRUser cannot access the patch directory, so just show the contents of
        // the patch-info file we create before, instead of monitoring the patch directory.
        QString msg;
        QFileInfo info;
//        PatchState newState = patchStateUnknown;
        PatchIcon iconIndex(patchIconNone);
        QSystemTrayIcon::MessageIcon msgIndex(QSystemTrayIcon::NoIcon);

        info.setFile(getTempLocation(),"patchstatus.txt");
        QFile f(info.filePath());
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&f);
            msg = stream.readAll();
            if (msg.startsWith("No Patch installed",Qt::CaseInsensitive))
            {
                iconIndex = patchIconNone;
                msgIndex = QSystemTrayIcon::Information;
            }
            else if (msg.startsWith("Patch installed",Qt::CaseInsensitive))
            {
                iconIndex = patchIconInstalled;
                msgIndex = QSystemTrayIcon::Warning;
            }
            else if (msg.contains("Applied patch",Qt::CaseInsensitive))
            {
                iconIndex = patchIconInstalled;
                msgIndex = QSystemTrayIcon::Warning;
            }
            else
            {
                msg.append("\n\nCould not interpret status info file");
                msgIndex = QSystemTrayIcon::Warning;
                iconIndex = patchIconError;
            }

        }
        else
        {
            if (!msg.isEmpty())
                msg.append("\n\n");
            msg.append("Could not read the status info file: ");
            msg.append(info.filePath());
            msgIndex = QSystemTrayIcon::Warning;
            iconIndex = patchIconError;
        }

        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(msgIndex);
        trayIcon->showMessage("Patch Status", msg, icon, 30 * 1000);
        setIcon(iconIndex);
        bodyLabel->setText(msg);
    }
}
//! [0]

//! [1]
void Window::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
//    maximizeAction->setEnabled(!isMaximized());
    restoreAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
}
//! [1]

//! [2]
void Window::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
/*        QMessageBox::information(this, tr("PatchMonitor"),
                                 tr("The program will keep running in the "
                                    "system tray. To terminate the program, "
                                    "choose <b>Quit</b> in the context menu "
                                    "of the system tray entry."));*/
        hide();
        event->ignore();
    }
}
//! [2]

//! [3]
void Window::setIcon(PatchIcon index)
{
    static const char* icons[] =
    {
        ":/images/patch-green.png",
        ":/images/patch.png",
        ":/images/patch-red.png"
    };

    static const char* tips[] =
    {
        "No patch installed",
        "Patch installed",
        "Patch state unknown"
    };

    QIcon icon(icons[index]);
    trayIcon->setIcon(icon);
    setWindowIcon(icon);

    trayIcon->setToolTip(tips[index]);
}
//! [3]

//! [4]
void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        break;

    case QSystemTrayIcon::DoubleClick:
        show();
//        iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1) % iconComboBox->count());
        break;
    case QSystemTrayIcon::MiddleClick:
//        showMessage();
        break;
    default:
        ;
    }
}
//! [4]

//! [5]
void Window::folderChanged(const QString& folder)
{
    // Note: these events will only occur in admin/gyrotest mode.
    // Regular user don't have access to the path folder, so a copy of the info is
    // used in the directory of the exe itself.

    QFileInfo info;
    PatchState newState = patchStateUnknown;
    QDir dir(folder);
    PatchIcon iconIndex(patchIconNone);
    QSystemTrayIcon::MessageIcon msgIndex(QSystemTrayIcon::NoIcon);
    QString msg;

    if (dir.count()==0)
    {
        newState = patchStateNone;
    }
    else
    {
        info.setFile(dir,"scanner.exe");
        if (info.exists())
        {
            info.setFile(dir,"_smart_patch_info_.txt");
            if (info.exists())
                newState = patchStateSmart;
            else
                newState = patchStateRegular;
        }
        else
            newState = patchStateNone;
    }

    if (newState!=patchState)
    {
        switch (newState)
        {
        case patchStateNone:
        {
            iconIndex = patchIconNone;
            msgIndex = QSystemTrayIcon::Information;
            msg = "No Patch installed";
            break;
        }

        case patchStateRegular:
            msg = "Patch installed";
            iconIndex = patchIconInstalled;
            msgIndex = QSystemTrayIcon::Warning;
            break;

        case patchStateSmart:
        {
            QFile f(info.filePath());
            int iRetry=0;
            #define MAX_RETRY 10
            // try reading the file (max 10 times; worst case 1 sec delay)
            for (iRetry=0;iRetry<MAX_RETRY;iRetry++)
            {
                if (f.open(QIODevice::ReadOnly | QIODevice::Text))
                    break;
                else
                    QThread::currentThread()->wait(100);
            }
            if (iRetry<MAX_RETRY)
            {
                QTextStream stream(&f);
                msg = stream.readAll();
                msg.replace("\n\n","\n"); // philips just keeps struggling with the empty lines...
                iconIndex = patchIconInstalled;
            }
            else
            {
                msg=QString("%1 is available but unreadable").arg(info.filePath());
                iconIndex = patchIconError;
            }
            msgIndex = QSystemTrayIcon::Warning;
            break;
        }

        default:
        case patchStateUnknown:
            msg = "Unknown Patch State";
            iconIndex = patchIconError;
            msgIndex = QSystemTrayIcon::Critical;
            break;
        }

        // Also write the message to a separate file which is accessible for a regular MRUser
        info.setFile(getTempLocation(),"patchstatus.txt");
        QFile f(info.filePath());
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&f);
            stream << msg;
        }
        else
        {
            msg.append("\n\nCould not create the status info file: ");
            msg.append(info.filePath());
            msgIndex = QSystemTrayIcon::Warning;
        }

        patchState=newState;

        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(msgIndex);
        trayIcon->showMessage("Patch Status", msg, icon, 30 * 1000);
        setIcon(iconIndex);
        bodyLabel->setText(msg);
    }
}
//! [5]

//! [6]
void Window::messageClicked()
{
    show();
/*    QMessageBox::information(0, tr("PatchMonitor"),
                             tr("Sorry, I already gave what help I could.\n"
                                "Maybe you should try asking Paul?"));*/
}

void Window::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

//    maximizeAction = new QAction(tr("Ma&ximize"), this);
//    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("&Show"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Window::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
//    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

QString Window::getTempLocation() const
{
    if (QFileInfo("E:/Export").exists())
        return QString("E:/Export"); // normally use this path because this is the only location that can be accessed by all users
    else
        return QDir::tempPath();
}


#endif
