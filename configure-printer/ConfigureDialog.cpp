/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti
    dantti12@gmail.com

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ConfigureDialog.h"
#include "PrinterPage.h"

#include "ModifyPrinter.h"
#include "PrinterBehavior.h"
#include "PrinterOptions.h"

#include "Debug.h"
#include "KCupsRequest.h"

#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KWindowConfig>

#include <QList>
#include <QPointer>
#include <QPushButton>

Q_DECLARE_METATYPE(QList<int>)

ConfigureDialog::ConfigureDialog(const QString &destName, bool isClass, QWidget *parent) :
    KPageDialog(parent)
{
    setFaceType(List);
    setModal(false);
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    setWindowTitle(destName);
    setWindowIcon(QIcon::fromTheme(QLatin1String("configure")));
    enableButtonApply(false);
    // Needed so we have our dialog size saved
    setAttribute(Qt::WA_DeleteOnClose);

    QStringList attr;
    KPageWidgetItem *page;

    modifyPrinter = new ModifyPrinter(destName, isClass, this);
    auto printerBehavior = new PrinterBehavior(destName, isClass, this);
    attr << modifyPrinter->neededValues();
    attr << printerBehavior->neededValues();
    attr << KCUPS_PRINTER_TYPE; // needed to know if it's a remote printer
    attr << KCUPS_PRINTER_MAKE_AND_MODEL;

    KCupsPrinter printer;
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->getPrinterAttributes(destName, isClass, attr);
    request->waitTillFinished();
    if (!request) {
        return;
    }
    if (!request->hasError() && !request->printers().isEmpty()){
        printer = request->printers().first();
    }
//    qCDebug(PM_CONFIGURE_PRINTER) << "VALUES" << printer.argument();
//    qCDebug(PM_CONFIGURE_PRINTER) << "marker" << values["marker-levels"].value<QList<int> >();

    request->deleteLater();

    //     qCDebug(PM_CONFIGURE_PRINTER) << values;
    if (printer.type() & CUPS_PRINTER_LOCAL) {
        qCDebug(PM_CONFIGURE_PRINTER) << "CUPS_PRINTER_LOCAL";
    }
    isClass = printer.isClass();
    bool isRemote = false;
    if (printer.type() & CUPS_PRINTER_REMOTE) {
        qCDebug(PM_CONFIGURE_PRINTER) << "CUPS_PRINTER_REMOTE";
        isRemote = true;
    }
    if (printer.type() & CUPS_PRINTER_BW) {
        qCDebug(PM_CONFIGURE_PRINTER) << "CUPS_PRINTER_BW";
    }
    if (printer.type() & CUPS_PRINTER_COLOR) {
        qCDebug(PM_CONFIGURE_PRINTER) << "CUPS_PRINTER_COLOR";
    }
    if (printer.type() & CUPS_PRINTER_MFP) {
        qCDebug(PM_CONFIGURE_PRINTER) << "CUPS_PRINTER_MFP";
    }

    modifyPrinter->setRemote(isRemote);
    modifyPrinter->setValues(printer);
    page = new KPageWidgetItem(modifyPrinter, i18n("Modify Printer"));
    page->setHeader(i18n("Configure"));
    page->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    // CONNECT this signal ONLY to the first Page
    connect(modifyPrinter, &ModifyPrinter::changed, this, &ConfigureDialog::enableButtonApply);
    addPage(page);

    if (!isClass) {
        // At least on localhost:631 modify printer does not show printer options
        // for classes
        printerOptions = new PrinterOptions(destName, isClass, isRemote, this);
        page = new KPageWidgetItem(printerOptions, i18n("Printer Options"));
        page->setHeader(i18n("Set the Default Printer Options"));
        page->setIcon(QIcon::fromTheme(QLatin1String("view-pim-tasks")));
        addPage(page);
        connect(modifyPrinter, &ModifyPrinter::ppdChanged, this, &ConfigureDialog::ppdChanged);
        modifyPrinter->setCurrentMake(printerOptions->currentMake());
        modifyPrinter->setCurrentMakeAndModel(printerOptions->currentMakeAndModel());
    }

    printerBehavior->setRemote(isRemote);
    printerBehavior->setValues(printer);
    page = new KPageWidgetItem(printerBehavior, i18n("Banners, Policies and Allowed Users"));
    page->setHeader(i18n("Banners, Policies and Allowed Users"));
    page->setIcon(QIcon::fromTheme(QLatin1String("feed-subscribe")));
    addPage(page);

    // connect this after ALL pages were added, otherwise the slot will be called
    connect(this, &ConfigureDialog::currentPageChanged, this, &ConfigureDialog::currentPageChangedSlot);

    KConfigGroup group(KSharedConfig::openConfig(QLatin1String("print-manager")), "ConfigureDialog");
    KWindowConfig::restoreWindowSize(windowHandle(), group);

    connect(buttonBox(), &QDialogButtonBox::clicked, this, &ConfigureDialog::slotButtonClicked);
}

void ConfigureDialog::ppdChanged()
{
    printerOptions->reloadPPD();
    modifyPrinter->setCurrentMake(printerOptions->currentMake());
    modifyPrinter->setCurrentMakeAndModel(printerOptions->currentMakeAndModel());
}

ConfigureDialog::~ConfigureDialog()
{
    KConfigGroup group(KSharedConfig::openConfig(QLatin1String("print-manager")), "ConfigureDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);
}

void ConfigureDialog::currentPageChangedSlot(KPageWidgetItem *current, KPageWidgetItem *before)
{
    auto currentPage = qobject_cast<PrinterPage*>(current->widget());
    auto beforePage = qobject_cast<PrinterPage*>(before->widget());

    qCDebug(PM_CONFIGURE_PRINTER) << "currentPageChanged" << beforePage << currentPage;
    // Check if the before page has changes
    savePage(beforePage);
    if (beforePage) {
        disconnect(beforePage, &PrinterPage::changed, this, &ConfigureDialog::enableButtonApply);
    }

    // connect the changed signal to the new page and check if it has changes
    connect(currentPage, &PrinterPage::changed, this, &ConfigureDialog::enableButtonApply);
    enableButtonApply(currentPage->hasChanges());
}

void ConfigureDialog::enableButtonApply(bool enable)
{
    qDebug() << Q_FUNC_INFO << enable << sender();
    button(QDialogButtonBox::QDialogButtonBox::Apply)->setEnabled(enable);
}

void ConfigureDialog::slotButtonClicked(QAbstractButton * pressedButton)
{
    auto page = qobject_cast<PrinterPage *>(currentPage()->widget());
    if (pressedButton == button(QDialogButtonBox::Ok) ||
        pressedButton == button(QDialogButtonBox::Apply)) {
        page->save();
    }
}

void ConfigureDialog::closeEvent(QCloseEvent *event)
{
    auto page = qobject_cast<PrinterPage*>(currentPage()->widget());
    if (savePage(page)) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool ConfigureDialog::savePage(PrinterPage *page)
{
    if (page->hasChanges()) {
        int ret;
        ret = KMessageBox::warningYesNoCancel(this,
                                               i18n("The current page has changes.\n"
                                                    "Do you want to save them?"));
        if (ret == KMessageBox::Yes) {
            page->save();
        } else if (ret == KMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

#include "moc_ConfigureDialog.cpp"
