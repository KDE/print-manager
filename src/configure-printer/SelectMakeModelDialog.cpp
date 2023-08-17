/*
    SPDX-FileCopyrightText: 2014 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SelectMakeModelDialog.h"

#include <QVBoxLayout>
#include <QPushButton>

#include <KLocalizedString>
#include <KPixmapSequenceOverlayPainter>
#include <KWindowConfig>
#include <KSharedConfig>
#include <KIconLoader>
#include <KPixmapSequence>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <KPixmapSequenceLoader>
#endif

#include "Debug.h"

SelectMakeModelDialog::SelectMakeModelDialog(const QString &make, const QString &makeModel, QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(i18n("Select a Driver"));

    auto layout = new QVBoxLayout(this);

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
    auto button = m_bbox->button(QDialogButtonBox::Help);
    button->setFlat(true);
    button->setEnabled(false);
    button->setIcon(QIcon());
    button->setText(QString());

    // Setup the busy cursor
    auto busySeq = new KPixmapSequenceOverlayPainter(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    busySeq->setSequence(KIconLoader::global()->loadPixmapSequence(QLatin1String("process-working"), KIconLoader::SizeSmallMedium));
#else
    busySeq->setSequence(KPixmapSequenceLoader::load(QLatin1String("process-working"), KIconLoader::SizeSmallMedium));
#endif
    busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    busySeq->setWidget(button);
    busySeq->start();
    connect(m_widget, &SelectMakeModel::changed, busySeq, &KPixmapSequenceOverlayPainter::stop);
    qCDebug(PM_CONFIGURE_PRINTER) << make << makeModel;

    // restore dlg size
    KConfigGroup group(KSharedConfig::openConfig(QLatin1String("print-manager")), "PPDDialog");
    KWindowConfig::restoreWindowSize(windowHandle(), group);

    // set data
    m_widget->setMakeModel(make, makeModel);
}

SelectMakeModelDialog::~SelectMakeModelDialog()
{
    // save dlg size
    KConfigGroup configGroup(KSharedConfig::openConfig(QLatin1String("print-manager")), "PPDDialog");
    KWindowConfig::saveWindowSize(windowHandle(), configGroup);
}

SelectMakeModel *SelectMakeModelDialog::mainWidget() const
{
    return m_widget;
}

#include "moc_SelectMakeModelDialog.cpp"
