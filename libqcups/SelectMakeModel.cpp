/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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

#include "SelectMakeModel.h"
#include "ui_SelectMakeModel.h"

#include "PPDModel.h"

#include "KCupsRequest.h"

#include <QStandardItemModel>
#include <QLineEdit>
#include <QItemSelection>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusMetaType>

#include <KMessageBox>
#include <KPixmapSequence>
#include <KDebug>

// Marshall the MyStructure data into a D-Bus argument
QDBusArgument &operator<<(QDBusArgument &argument, const DriverMatch &driverMatch)
{
    argument.beginStructure();
    argument << driverMatch.ppd << driverMatch.match;
    argument.endStructure();
    return argument;
}

// Retrieve the MyStructure data from the D-Bus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, DriverMatch &driverMatch)
{
    argument.beginStructure();
    argument >> driverMatch.ppd >> driverMatch.match;
    argument.endStructure();
    return argument;
}

SelectMakeModel::SelectMakeModel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectMakeModel),
    m_ppdRequest(0),
    m_gotBestDrivers(false)
{
    ui->setupUi(this);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(0, 2);

    m_sourceModel = new PPDModel(this);

    ui->makeView->setModel(m_sourceModel);
    // Updates the PPD view to the selected Make
    connect(ui->makeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            ui->ppdsLV, SLOT(setRootIndex(QModelIndex)));

    // Clear the PPD view selection, so the Next/Finish button gets disabled
    connect(ui->makeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            ui->ppdsLV->selectionModel(), SLOT(clearSelection()));

    ui->ppdsLV->setModel(m_sourceModel);
    connect(m_sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(checkChanged()));

    // Make sure we update the Next/Finish button if a PPD is selected
    connect(ui->ppdsLV->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(checkChanged()));    

    qDBusRegisterMetaType<DriverMatch>();
    qDBusRegisterMetaType<DriverMatchList>();

    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(ui->ppdsLV->viewport());

}

SelectMakeModel::~SelectMakeModel()
{
    delete ui;
}

void SelectMakeModel::setDeviceInfo(const QString &deviceId, const QString &makeAndModel, const QString &deviceUri)
{
    kDebug() << "===================================" << deviceId;
    m_gotBestDrivers = false;
    // Get the best drivers
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("/org/fedoraproject/Config/Printing"),
                                             QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("GetBestDrivers"));
    message << deviceId;
    message << makeAndModel;
    message << deviceUri;
    QDBusConnection::sessionBus().callWithCallback(message,
                                                   this,
                                                   SLOT(getBestDriversFinished(QDBusMessage)),
                                                   SLOT(getBestDriversFailed(QDBusError,QDBusMessage)));

    if (!m_ppdRequest) {
        m_ppdRequest = new KCupsRequest;
        m_ppdRequest->getPPDS();
        connect(m_ppdRequest, SIGNAL(finished()), this, SLOT(ppdsLoaded()));
    }
}

void SelectMakeModel::setMakeModel(const QString &make, const QString &makeAndModel)
{
//    if (!m_request) {
//        m_request = new KCupsRequest;
//        m_request->getPPDS();
//        connect(m_request, SIGNAL(finished()), this, SLOT(ppdsLoaded()));
//        m_make = make;
//        m_makeAndModel = makeAndModel;
//        m_busySeq->start();
//    } else {
//        m_busySeq->stop();
//        if (!makeAndModel.isEmpty()) {
//            // Tries to find the current PPD and select it
//            for (int i = 0; i < m_model->rowCount(); i++) {
//                QString modelMakeAndModel;
//                modelMakeAndModel = m_model->index(i, 0).data(PPDModel::PPDMakeAndModel).toString();
//                if (modelMakeAndModel == makeAndModel) {
//                    ui->ppdsLV->setCurrentIndex(m_model->index(i, 0));
//                    break;
//                }
//            }
//        }
//    }
}

void SelectMakeModel::ppdsLoaded()
{
    if (m_ppdRequest->hasError()) {
        kWarning() << "Failed to get PPDs" << m_ppdRequest->errorMsg();
        m_ppdRequest = 0;
    } else {
        m_ppds = m_ppdRequest->ppds();

        // Try to show the PPDs
        setModelData();
    }
    sender()->deleteLater();
}

void SelectMakeModel::checkChanged()
{
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = ui->ppdsLV->selectionModel()->selection();

    // enable or disable the job action buttons if something is selected
    emit changed(!selection.indexes().isEmpty());
    kDebug() << sender() << selection.indexes().isEmpty();
    if (!selection.indexes().isEmpty()) {
        QModelIndex index = selection.indexes().first();
        m_selectedMakeAndModel = index.data(PPDModel::PPDMakeAndModel).toString();
        m_selectedPPDName = index.data(PPDModel::PPDName).toString();
        emit changed(m_makeAndModel != m_selectedMakeAndModel);
    } else {
        m_selectedMakeAndModel.clear();
        m_selectedPPDName.clear();
        emit changed(false);
        ui->makeView->selectionModel()->setCurrentIndex(m_sourceModel->index(0, 0),
                                                        QItemSelectionModel::SelectCurrent);
    }
}

QString SelectMakeModel::selectedPPDName() const
{
    return m_selectedPPDName;
}

QString SelectMakeModel::selectedMakeAndModel() const
{
    return m_selectedMakeAndModel;
}

void SelectMakeModel::getBestDriversFinished(const QDBusMessage &message)
{
    if (message.type() == QDBusMessage::ReplyMessage && message.arguments().size() == 1) {
        QDBusArgument argument = message.arguments().first().value<QDBusArgument>();
        m_driverMatchList = qdbus_cast<DriverMatchList>(argument);

        foreach (const DriverMatch &driverMatch, m_driverMatchList) {
            kDebug() << driverMatch.ppd << driverMatch.match;
        }
    } else {
        kWarning() << "Unexpected message" << message;
    }
    m_gotBestDrivers = true;
    setModelData();
}

void SelectMakeModel::getBestDriversFailed(const QDBusError &error, const QDBusMessage &message)
{
    kWarning() << "Failed to get best drivers" << error << message;

    // Show the PPDs anyway
    m_gotBestDrivers = true;
    setModelData();
}

void SelectMakeModel::setModelData()
{
    if (!m_ppds.isEmpty() && m_gotBestDrivers) {
        m_sourceModel->setPPDs(m_ppds, m_driverMatchList);
        checkChanged();
    }
}
