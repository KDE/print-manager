/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ChooseLpd.h"
#include "ui_ChooseLpd.h"

#include <KCupsRequest.h>
#include <KLocalizedString>

#include <QPainter>

#include <QUrl>

ChooseLpd::ChooseLpd(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::ChooseLpd),
    m_isValid(false)
{
    ui->setupUi(this);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));
}

ChooseLpd::~ChooseLpd()
{
    delete ui;
}

void ChooseLpd::on_addressLE_textChanged(const QString &text)
{
    Q_UNUSED(text)
//    qDebug() << text;
}

void ChooseLpd::setValues(const QVariantHash &args)
{
    m_args = args;
    const QString deviceUri = args[KCUPS_DEVICE_URI].toString();
    if (deviceUri.contains(QLatin1Char('/'))) {
        m_isValid = false;
        return;
    }
    m_isValid = true;

    ui->addressLE->setText(deviceUri);
    ui->addressLE->setFocus();
}

QVariantHash ChooseLpd::values() const
{
    QVariantHash ret = m_args;
    ret[KCUPS_DEVICE_URI] = static_cast<QString>(QLatin1String("lpd://") + ui->addressLE->text());
    return ret;
}

bool ChooseLpd::canProceed() const
{
    bool allow = false;
    if (!ui->addressLE->text().isEmpty()) {
        const QUrl url = QUrl(QLatin1String("lpd://") + ui->addressLE->text());
        allow = url.isValid();
    }
    return allow;
}

bool ChooseLpd::isValid() const
{
    return m_isValid;
}

void ChooseLpd::checkSelected()
{
//     emit allowProceed(!devicesLV->selectionModel()->selection().isEmpty());
}

#include "moc_ChooseLpd.cpp"
