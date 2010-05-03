/***************************************************************************
 *   Copyright (C) 2010 Daniel Nicoletti <dantti85-pk@yahoo.com.br>        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "PrintQueueInterface.h"
#include "printqueueadaptor.h"

#include "PrintQueueUi.h"
#include "QCups.h"
#include <cups/cups.h>

#include <QtDBus/QDBusConnection>
#include <QtCore/QTimer>
#include <KWindowSystem>
#include <KDialog>

#include <KDebug>

PrintQueueInterface::PrintQueueInterface(QObject *parent)
    : QObject(parent)
{
    kDebug() << "Creating Helper";
    (void) new PrintQueueAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService("org.kde.PrintQueue")) {
        kDebug() << "another helper is already running";
        return;
    }

    if (!QDBusConnection::sessionBus().registerObject("/", this)) {
        kDebug() << "unable to register service interface to dbus";
        return;
    }

    // setup the timer that updates the UIs
    m_updateUi = new QTimer(this);
    m_updateUi->setInterval(1000);
    m_updateUi->start();
}

PrintQueueInterface::~PrintQueueInterface()
{
}

void PrintQueueInterface::ShowQueue(const QString &destName)
{
    if (destName.isEmpty()) {
//         emit quit();
        return;
    }

    if(!m_uis.contains(destName)) {
        QCups::ReturnArguments dests;
        QStringList requestAttr;
        requestAttr << "printer-name"
                    << "printer-type";
        // Get destinations with these attributes
        QCups::Result *ret = QCups::getDests(-1, requestAttr);
        ret->waitTillFinished();
        dests = ret->result();
        ret->deleteLater();

        bool found = false;
        bool isClass = false;
        for (int i = 0; i < dests.size(); i++) {
            if (dests.at(i)["printer-name"] == destName) {
                isClass = dests.at(i)["printer-type"].toInt() & CUPS_PRINTER_CLASS;
                found = true;
                break;
            }
        }

        if (found) {
            PrintQueueUi *ui = new PrintQueueUi(destName, isClass);
            KDialog *dlg = new KDialog;
            dlg->setWindowIcon(KIcon("printer").pixmap(32));
            dlg->setWindowTitle(ui->windowTitle());
            dlg->setButtons(0);
            dlg->setMainWidget(ui);
            dlg->setSizeGripEnabled(true);
            (void)dlg->minimumSizeHint(); //Force the dialog to be laid out now
            dlg->layout()->setContentsMargins(0,0,0,0);
            connect(m_updateUi, SIGNAL(timeout()),
                    ui, SLOT(update()));
            connect(dlg, SIGNAL(finished()),
                    this, SLOT(RemoveQueue()));
            connect(ui, SIGNAL(windowTitleChanged(const QString &)),
                    dlg, SLOT(setWindowTitle(const QString &)));
            dlg->show();
            m_uis[destName] = dlg;

        } else {
            // if no destination was found and we aren't showing
            // a queue quit the app
            if (m_uis.isEmpty()) {
//                 emit quit();
            }
            return;
        }
    }
    KWindowSystem::forceActiveWindow(m_uis[destName]->winId());
}

void PrintQueueInterface::RemoveQueue()
{
    QWidget *ui = qobject_cast<QWidget*>(sender());
    m_uis.remove(m_uis.key(ui));
}

#include "PrintQueueInterface.moc"
