/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrinterDescription.h"
#include "ui_PrinterDescription.h"

#include <KCupsPrinter.h>

#include <QPainter>
#include <QPointer>
#include <QProgressBar>
#include <QLabel>
#include <QMenu>
#include <QProcess>

#include <KIO/CommandLauncherJob>

#define PRINTER_ICON_SIZE 128

Q_DECLARE_METATYPE(QList<int>)

PrinterDescription::PrinterDescription(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrinterDescription)
{
    ui->setupUi(this);
    m_layoutEnd = ui->formLayout->count();

    // loads the standard key icon
    m_printerIcon = QIcon::fromTheme(QStringLiteral("printer")).pixmap(PRINTER_ICON_SIZE);
    ui->iconL->setPixmap(m_printerIcon);

    auto menu = new QMenu(ui->maintenancePB);
    menu->addAction(ui->actionPrintTestPage);
    menu->addAction(ui->actionPrintSelfTestPage);
    menu->addAction(ui->actionCleanPrintHeads);
    ui->actionCleanPrintHeads->setVisible(false);
    ui->actionPrintSelfTestPage->setVisible(false);
    ui->maintenancePB->setMenu(menu);
    ui->errorMessage->setWordWrap(true);
    ui->errorMessage->setMessageType(KMessageWidget::Error);
    ui->errorMessage->hide();
}

PrinterDescription::~PrinterDescription()
{
    delete ui;
}

void PrinterDescription::on_openQueuePB_clicked()
{
    auto job = new KIO::CommandLauncherJob(QStringLiteral("kde-print-queue"), { m_destName });
    job->start();
}

void PrinterDescription::on_defaultCB_clicked()
{
    ui->defaultCB->setDisabled(true);
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterDescription::requestFinished);
    request->setDefaultPrinter(m_destName);
}

void PrinterDescription::on_sharedCB_clicked()
{
    ui->sharedCB->setDisabled(true);
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterDescription::requestFinished);
    request->setShared(m_destName, m_isClass, ui->sharedCB->isChecked());
}

void PrinterDescription::on_rejectPrintJobsCB_clicked()
{
    ui->rejectPrintJobsCB->setDisabled(true);
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterDescription::requestFinished);
    if (ui->rejectPrintJobsCB->isChecked()) {
        request->rejectJobs(m_destName);
    } else {
        request->acceptJobs(m_destName);
    }
}

void PrinterDescription::setPrinterIcon(const QIcon &icon)
{
    ui->iconL->setPixmap(icon.pixmap(PRINTER_ICON_SIZE, PRINTER_ICON_SIZE));
}

void PrinterDescription::setDestName(const QString &name, const QString &description, bool isClass, bool singlePrinter)
{
    m_destName = name;

    m_markerData.clear();
    if (m_isClass != isClass) {
        m_isClass = isClass;
        ui->sharedCB->setText(m_isClass ? i18n("Share this class") : i18n("Share this printer"));
    }

    ui->nameMsgL->setText(name);
    if (!description.isEmpty() && description != ui->printerNameL->text()) {
        ui->printerNameL->setText(description);
    } else if (description.isEmpty() && name != ui->printerNameL->text()) {
        ui->printerNameL->setText(name);
    }

    ui->statusL->setVisible(singlePrinter);
    ui->nameL->setVisible(singlePrinter);
    ui->nameMsgL->setVisible(singlePrinter);
}

void PrinterDescription::setDestStatus(const QString &status)
{
    ui->statusL->setText(status);
}

void PrinterDescription::setLocation(const QString &location)
{
    ui->locationMsgL->setText(location);
}

void PrinterDescription::setKind(const QString &kind)
{
    ui->kindMsgL->setText(kind);
}

void PrinterDescription::setIsDefault(bool isDefault)
{
    ui->defaultCB->setEnabled(!isDefault);
    ui->defaultCB->setChecked(isDefault);
}

void PrinterDescription::setIsShared(bool isShared)
{
    m_isShared = isShared;
    if (m_globalShared) {
        ui->sharedCB->setChecked(isShared);
    } else {
        ui->sharedCB->setChecked(false);
    }
    ui->sharedCB->setEnabled(m_globalShared);
}

void PrinterDescription::setAcceptingJobs(bool accepting)
{
    ui->rejectPrintJobsCB->setEnabled(true);
    ui->rejectPrintJobsCB->setChecked(!accepting);
}

void PrinterDescription::setCommands(const QStringList &commands)
{
    // On the first time this method runs the list
    // can be empty, so keep all objects initialized on the
    // constructor
    if (m_commands != commands) {
        m_commands = commands;

        ui->actionCleanPrintHeads->setVisible(commands.contains(QLatin1String("Clean")));
        ui->actionPrintSelfTestPage->setVisible(commands.contains(QLatin1String("PrintSelfTestPage")));

        // TODO if the printer supports ReportLevels
        // we should probably probe for them
        // commands.contains("ReportLevels")
    }
}

void PrinterDescription::setMarkers(const QVariantHash &data)
{
    // Remove old progress bars
    while (ui->formLayout->count() > m_layoutEnd) {
        ui->formLayout->takeAt(ui->formLayout->count() - 1)->widget()->deleteLater();
    }

    int size = data[KCUPS_MARKER_NAMES].toStringList().size();
    if (size != data[KCUPS_MARKER_LEVELS].value<QList<int> >().size() ||
            size != data[KCUPS_MARKER_COLORS].toStringList().size() ||
            size != data[KCUPS_MARKER_TYPES].toStringList().size()) {
        return;
    }

    // Create a colored progress bar for each marker
    for (int i = 0; i < size; i++) {
        if (data[KCUPS_MARKER_TYPES].toStringList().at(i) == QLatin1String("unknown")) {
            continue;
        }
        auto pogressBar = new QProgressBar;
        pogressBar->setValue(data[KCUPS_MARKER_LEVELS].value<QList<int> >().at(i));
        pogressBar->setTextVisible(false);
        pogressBar->setMaximumHeight(15);
        QPalette palette = pogressBar->palette();
        palette.setColor(QPalette::Active,
                         QPalette::Highlight,
                         QColor(data[KCUPS_MARKER_COLORS].toStringList().at(i)));
        palette.setColor(QPalette::Inactive,
                         QPalette::Highlight,
                         QColor(data[KCUPS_MARKER_COLORS].toStringList().at(i)).lighter());
        pogressBar->setPalette(palette);
        auto label = new QLabel(data[KCUPS_MARKER_NAMES].toStringList().at(i), this);
        ui->formLayout->addRow(label, pogressBar);
    }
}

void PrinterDescription::on_actionPrintTestPage_triggered(bool checked)
{
    Q_UNUSED(checked)

    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterDescription::requestFinished);
    request->printTestPage(m_destName, m_isClass);
}

void PrinterDescription::on_actionCleanPrintHeads_triggered(bool checked)
{
    Q_UNUSED(checked)

    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterDescription::requestFinished);
    request->printCommand(m_destName, QLatin1String("Clean all"), i18n("Clean Print Heads"));
}

void PrinterDescription::on_actionPrintSelfTestPage_triggered(bool checked)
{
    Q_UNUSED(checked)

    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterDescription::requestFinished);
    request->printCommand(m_destName, QLatin1String("PrintSelfTestPage"), i18n("Print Self-Test Page"));
}

void PrinterDescription::requestFinished(KCupsRequest *request)
{
    if (request && request->hasError()) {
        ui->errorMessage->setText(i18n("Failed to perform request: %1", request->errorMsg()));
        ui->errorMessage->animatedShow();
        Q_EMIT updateNeeded();
    }
}

QString PrinterDescription::destName() const
{
    return m_destName;
}

void PrinterDescription::enableShareCheckBox(bool enable)
{
    m_globalShared = enable;
    setIsShared(m_isShared);
}

void PrinterDescription::on_configurePB_clicked()
{
    QProcess::startDetached(QLatin1String("configure-printer"), {m_destName});
}

#include "moc_PrinterDescription.cpp"
