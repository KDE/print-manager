/***************************************************************************
 *   Copyright (C) 2010-2018 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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

#include "PageChoosePPD.h"
#include "ui_PageChoosePPD.h"

#include "DevicesModel.h"

#include <SelectMakeModel.h>
#include <KCupsRequest.h>

#include <QFileInfo>
#include <QFile>
#include <QStringBuilder>
#include <QDebug>
#include <QUrl>
#include <QTemporaryFile>

PageChoosePPD::PageChoosePPD(const QVariantHash &args, QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::PageChoosePPD),
    m_isValid(false)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));

    m_layout = new QStackedLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    ui->gridLayout->addLayout(m_layout, 1, 3);
    m_selectMM = new SelectMakeModel(this);
    connect(m_selectMM, &SelectMakeModel::changed, this, &PageChoosePPD::checkSelected);
    m_layout->addWidget(m_selectMM);

    // Setup the busy cursor
    connect(m_selectMM, &SelectMakeModel::changed, this, &PageChoosePPD::notWorking);

    if (!args.isEmpty()) {
        // set our args
        setValues(args);
    }
}

PageChoosePPD::~PageChoosePPD()
{
    removeTempPPD();

    delete ui;
}

void PageChoosePPD::setValues(const QVariantHash &args)
{
    m_args = args;

    if (args[ADDING_PRINTER].toBool()) {

        qDebug() << args;
        working();
        removeTempPPD();
        QString deviceId = args[KCUPS_DEVICE_ID].toString();
        QString make;
        QString makeAndModel = args[KCUPS_DEVICE_MAKE_AND_MODEL].toString();
        QString deviceURI = args[KCUPS_DEVICE_URI].toString();

        // If
        QUrl url(deviceURI % QStringLiteral(".ppd"));
        if (url.scheme() == QStringLiteral("ipp")) {
            auto tempFile = new QTemporaryFile;
            tempFile->setFileTemplate(QStringLiteral("print-manager-XXXXXX.ppd"));
            tempFile->open();
            url.setScheme(QStringLiteral("http"));
            if (url.port() < 0) {
                url.setPort(631);
            }
            qDebug() << deviceURI << url;
            KJob *job = KIO::file_copy(url,
                                      QUrl::fromLocalFile(tempFile->fileName()),
                                      -1,
                                      KIO::Overwrite | KIO::HideProgressInfo);
            job->setProperty("URI", deviceURI);
            connect(job, &KJob::result, this, &PageChoosePPD::resultJob);
        }

        // Get the make from the device id
        foreach (const QString &pair, deviceId.split(QLatin1Char(';'))) {
            if (pair.startsWith(QStringLiteral("MFG:"))) {
                make = pair.section(QLatin1Char(':'), 1);
                break;
            }
        }

        if (makeAndModel.isEmpty()) {
            // Get the model  from the device id
            foreach (const QString &pair, deviceId.split(QLatin1Char(';'))) {
                if (pair.startsWith(QStringLiteral("MDL:"))) {
                    // Build the make and model string
                    if (make.isNull()) {
                        makeAndModel = pair.section(QLatin1Char(':'), 1);
                    } else {
                        makeAndModel = make % QLatin1Char(' ') % pair.section(QLatin1Char(':'), 1);
                    }
                    break;
                }
            }
        }

        // if the device info is empty use the make and model
        // so we can have a nice name for the new printer on the next page
        if (!args.contains(KCUPS_DEVICE_INFO) && !makeAndModel.isEmpty()) {
            m_args[KCUPS_DEVICE_INFO] = makeAndModel;
        }

        m_selectMM->setDeviceInfo(deviceId, make, makeAndModel, deviceURI);
        m_isValid = true;
    } else {
        m_isValid = false;
    }
}

bool PageChoosePPD::isValid() const
{
    return m_isValid;
}

QVariantHash PageChoosePPD::values() const
{
    if (!isValid()) {
        return m_args;
    }

    QVariantHash ret = m_args;
    if (canProceed()) {
        if (!m_ppdFile.isNull()) {
            ret[FILENAME] = m_ppdFile;
        } else if (m_selectMM->isFileSelected()) {
            ret[FILENAME] = m_selectMM->selectedPPDFileName();
        } else {
            ret[PPD_NAME] = m_selectMM->selectedPPDName();
        }
    }
    return ret;
}

bool PageChoosePPD::canProceed() const
{
    // It can proceed if a PPD file (local or not) is provided    bool changed = false;
    bool allow = false;

    if (m_selectMM->isFileSelected()) {
        allow = !m_selectMM->selectedPPDFileName().isNull();
    } else if (!m_ppdFile.isNull()) {
        allow = true;
    } else {
        allow = !m_selectMM->selectedPPDName().isNull();
    }

    qDebug() << allow;
    return allow;
}

void PageChoosePPD::checkSelected()
{
    emit allowProceed(canProceed());
}

void PageChoosePPD::selectDefault()
{
}

void PageChoosePPD::resultJob(KJob *job)
{
    if (!job->error() && job->property("URI").toString() == m_args[KCUPS_DEVICE_URI].toString()) {
        auto fileCopyJob = qobject_cast<KIO::FileCopyJob*>(job);

        // Make sure this job is for the current device
        m_ppdFile = fileCopyJob->destUrl().toLocalFile();
        m_isValid = false;
        emit proceed();
    }
}

void PageChoosePPD::removeTempPPD()
{
    if (!m_ppdFile.isEmpty()) {
        QFile::remove(m_ppdFile);
        m_ppdFile.clear();
    }
}
