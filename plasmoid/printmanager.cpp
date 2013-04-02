/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti <dantti12@gmail.com>           *
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

#include "printmanager.h"

#include "PlasmoidConfig.h"

#include <QtGui/QGraphicsLinearLayout>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative>

#include <QDBusConnection>
#include <QDBusServiceWatcher>

#include <KConfigDialog>
#include <KCModuleProxy>

#include <Plasma/ToolTipManager>
#include <Plasma/ToolTipContent>
#include <Plasma/DeclarativeWidget>
#include <Plasma/Package>

#include <PrinterModel.h>
#include <PrinterSortFilterModel.h>
#include <PrintQueueModel.h>

PrintManager::PrintManager(QObject *parent, const QVariantList &args) :
    PopupApplet(parent, args),
    m_declarativeWidget(0)
{
    setHasConfigurationInterface(true);

    KGlobal::insertCatalog(QLatin1String("print-manager"));
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setActive(false);
    setPopupIcon("printer");

    m_printQueueModel = new PrintQueueModel(this);
}

PrintManager::~PrintManager()
{
}

void PrintManager::init()
{
    switch (formFactor()) {
    case Plasma::Horizontal:
    case Plasma::Vertical:
        Plasma::ToolTipManager::self()->registerWidget(this);
        break;
    default:
        Plasma::ToolTipManager::self()->unregisterWidget(this);
        break;
    }

    m_printQueueModel->init();

    PopupApplet::init();
}

QGraphicsWidget *PrintManager::graphicsWidget()
{
    if (!m_declarativeWidget) {
        m_declarativeWidget = new Plasma::DeclarativeWidget(this);
        m_declarativeWidget->engine()->rootContext()->setContextProperty("plasmoid", this);
        m_declarativeWidget->engine()->rootContext()->setContextProperty("jobsModel", m_printQueueModel);
        qmlRegisterType<PrinterModel>("org.kde.printmanager", 0, 1, "PrinterModel");
        qmlRegisterType<PrinterSortFilterModel>("org.kde.printmanager", 0, 1, "PrinterSortFilterModel");
        qmlRegisterType<PrintQueueModel>("org.kde.printmanager", 0, 1, "PrintQueueModel");

        Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
        Plasma::Package package(QString(), "org.kde.printmanager", structure);
        m_declarativeWidget->setQmlPath(package.filePath("mainscript"));
    }
    return m_declarativeWidget;
}

void PrintManager::toolTipAboutToShow()
{
    if (isPopupShowing()) {
        Plasma::ToolTipManager::self()->clearContent(this);
        return;
    }

    QString text;
    KIcon icon;
    if (m_printQueueModel->rowCount() == 0) {
        text = i18n("Print queue is empty");
        icon = KIcon("printer");
    } else {
        text = i18np("There is one print job in the queue",
                     "There are %1 print jobs in the queue",
                     m_printQueueModel->rowCount());
        icon = KIcon("printer-printing");
    }
    Plasma::ToolTipContent content(text,
                                   QString(),
                                   icon);

    Plasma::ToolTipManager::self()->setContent(this, content);
}

void PrintManager::setActive(bool active)
{
    if (active) {
        setStatus(Plasma::ActiveStatus);
    } else if (status() != Plasma::PassiveStatus && status() != Plasma::NeedsAttentionStatus) {
        setStatus(Plasma::PassiveStatus);
    }
}

void PrintManager::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        switch (formFactor()) {
        case Plasma::Horizontal:
        case Plasma::Vertical:
            Plasma::ToolTipManager::self()->registerWidget(this);
            break;
        default:
            Plasma::ToolTipManager::self()->unregisterWidget(this);
            break;
        }
    }
}

void PrintManager::createConfigurationInterface(KConfigDialog *parent)
{
    // TODO: Add printer filtering for plasmoid
    parent->addPage(new PlasmoidConfig(parent),
                    QLatin1String("Behavior"),
                    QLatin1String("preferences-other"),
                    i18n("Behavior"),
                    true);

    // Add the printer KCM to the applet's configdialog
    parent->addPage(new KCModuleProxy("kcm_printer_manager", parent),
                    i18n("Printers"),
                    QLatin1String("printer"),
                    i18n("Printers"),
                    false);

    parent->setMinimumSize(830,490);
    parent->resize(960, 600);

    // NOTE: Not needed not but probably when the plasmoid gets its own config
    //connect(parent, SIGNAL(applyClicked()), this, SLOT(saveConfiguration()));
    //connect(parent, SIGNAL(okClicked()), this, SLOT(saveConfiguration()));
}

#include "printmanager.moc"
