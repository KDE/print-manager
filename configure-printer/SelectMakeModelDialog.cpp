/***************************************************************************
 *   Copyright (C) 2014 Lukáš Tinkl <ltinkl@redhat.com>                    *
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
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include <QVBoxLayout>
#include <QPushButton>

#include <KLocalizedString>
#include <KPixmapSequenceOverlayPainter>
#include <KWindowConfig>
#include <KSharedConfig>
#include <KIconLoader>
#include <KPixmapSequence>

#include "SelectMakeModelDialog.h"
#include "Debug.h"

SelectMakeModelDialog::SelectMakeModelDialog(const QString &make, const QString &makeModel, QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(i18n("Select a Driver"));

    QVBoxLayout * layout = new QVBoxLayout(this);

    m_widget = new SelectMakeModel(this);
    layout->addWidget(m_widget);

    m_bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help,
                                  Qt::Horizontal, this);
    layout->addWidget(m_bbox);

    connect(m_bbox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_bbox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_widget, &SelectMakeModel::changed, m_bbox->button(QDialogButtonBox::Ok), &QPushButton::setEnabled);
    m_bbox->button(QDialogButtonBox::Ok)->setEnabled(false);

    // Configure the help button to be flat, disabled and empty
    QPushButton *button = m_bbox->button(QDialogButtonBox::Help);
    button->setFlat(true);
    button->setEnabled(false);
    button->setIcon(QIcon());
    button->setText(QString());

    // Setup the busy cursor
    KPixmapSequenceOverlayPainter *busySeq = new KPixmapSequenceOverlayPainter(this);
    busySeq->setSequence(KIconLoader::global()->loadPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    busySeq->setWidget(button);
    busySeq->start();
    connect(m_widget, &SelectMakeModel::changed, busySeq, &KPixmapSequenceOverlayPainter::stop);
    qCDebug(PM_CONFIGURE_PRINTER) << make << makeModel;

    // restore dlg size
    KConfigGroup group(KSharedConfig::openConfig("print-manager"), "PPDDialog");
    KWindowConfig::restoreWindowSize(windowHandle(), group);

    // set data
    m_widget->setMakeModel(make, makeModel);
}

SelectMakeModelDialog::~SelectMakeModelDialog()
{
    // save dlg size
    KConfigGroup configGroup(KSharedConfig::openConfig("print-manager"), "PPDDialog");
    KWindowConfig::saveWindowSize(windowHandle(), configGroup);
}

SelectMakeModel *SelectMakeModelDialog::mainWidget() const
{
    return m_widget;
}
