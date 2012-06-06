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
    m_request(0)
{
    ui = new Ui::SelectMakeModel;
    ui->setupUi(this);

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
    // Get the best drivers
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("/org/fedoraproject/Config/Printing"),
                                             QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("GetBestDrivers"));
    message << deviceId;
    message << makeAndModel;
    message << deviceUri;
    QDBusReply<DriverMatchList> reply = QDBusConnection::sessionBus().call(message, QDBus::BlockWithGui);

    DriverMatchList drivers;
    if (reply.isValid()) {
        drivers = reply.value();
    }

    foreach (const DriverMatch &driverMatch, drivers) {
        kDebug() << driverMatch.ppd << driverMatch.match;
    }

    kDebug() << drivers.count() << reply.error();
}

void SelectMakeModel::setMakeModel(const QString &make, const QString &makeAndModel)
{
    if (!m_request) {
        m_request = new KCupsRequest;
        m_request->getPPDS();
        connect(m_request, SIGNAL(finished()), this, SLOT(ppdsLoaded()));
        m_make = make;
        m_makeAndModel = makeAndModel;
        m_busySeq->start();
    } else {
        m_busySeq->stop();
        ui->makeFilterKCB->setCurrentIndex(ui->makeFilterKCB->findText(make));
        if (!makeAndModel.isEmpty()) {
            // Tries to find the current PPD and select it
            for (int i = 0; i < m_model->rowCount(); i++) {
                QString modelMakeAndModel;
                modelMakeAndModel = m_model->index(i, 0).data(PPDModel::PPDMakeAndModel).toString();
                if (modelMakeAndModel == makeAndModel) {
                    ui->ppdsLV->setCurrentIndex(m_model->index(i, 0));
                    break;
                }
            }
        }
    }
}

void SelectMakeModel::ppdsLoaded()
{
    ReturnArguments ppds = m_request->ppds();
    PPDModel *sourceModel = new PPDModel(ppds, this);
    m_request->deleteLater(); // Do not make it be 0 since it takes a lot to load
    m_model = new QSortFilterProxyModel(this);
    m_model->setSourceModel(sourceModel);
    ui->ppdsLV->setModel(m_model);

    QStringList makes;
    for (int i = 0; i < ppds.size(); i++) {
        makes << ppds.at(i)["ppd-make"].toString();
    }
    makes.sort();
    makes.removeDuplicates();
    ui->makeFilterKCB->addItems(makes);

    QStandardItemModel *makeModel = new QStandardItemModel(this);
    foreach (const QString &make, makes) {
        QStandardItem *stdItem = new QStandardItem(make);
        makeModel->appendRow(stdItem);
    }
    ui->makeView->setModel(makeModel);

    connect(ui->ppdsLV->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(checkChanged()));
    // find the make and model desired
    setMakeModel(m_make, m_makeAndModel);
}

void SelectMakeModel::checkChanged()
{
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = ui->ppdsLV->selectionModel()->selection();
    // enable or disable the job action buttons if something is selected
    emit changed(!selection.indexes().isEmpty());
    if (!selection.indexes().isEmpty()) {
        QModelIndex index = selection.indexes().at(0);
        m_selectedMakeAndModel = index.data(PPDModel::PPDMakeAndModel).toString();
        m_selectedPPDName = index.data(PPDModel::PPDName).toString();
        emit changed(m_makeAndModel != m_selectedMakeAndModel);
    } else {
        m_selectedMakeAndModel.clear();
        m_selectedPPDName.clear();
        emit changed(false);
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

void SelectMakeModel::on_makeFilterKCB_editTextChanged(const QString &text)
{
    // We can't be sure if activated or current indexChanged signal
    // will be emmited before this signal.
    // So we check if the line edit was modified by the user to be 100% sure
    PPDModel *sourceModel = qobject_cast<PPDModel *>(m_model->sourceModel());
    if (ui->makeFilterKCB->lineEdit()->isModified()) {
        sourceModel->setMake(QString());
        m_model->setFilterRole(Qt::DisplayRole);
        m_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_model->setFilterFixedString(text);
    } else {
        sourceModel->setMake(text);
        m_model->setFilterRole(PPDModel::PPDMake);
        m_model->setFilterCaseSensitivity(Qt::CaseSensitive);
        m_model->setFilterFixedString(text);
    }
}


#include "SelectMakeModel.moc"
