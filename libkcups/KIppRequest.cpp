/***************************************************************************
 *   Copyright (C) 2010-2013 by Daniel Nicoletti                           *
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

#include "KIppRequest.h"
#include "KIppRequest_p.h"

#include <QStringBuilder>

#include <KDebug>

KIppRequest::KIppRequest() :
    d_ptr(new KIppRequestPrivate)
{
}

KIppRequest::KIppRequest(const KIppRequest &other) :
    d_ptr(new KIppRequestPrivate)
{
    *this = other;
}

KIppRequest::KIppRequest(ipp_op_t operation, const char *resource, const QString &filename) :
    d_ptr(new KIppRequestPrivate)
{
    Q_D(KIppRequest);

    d->operation = operation;
    d->resource = resource;
    d->filename = filename;

    // send our user name on the request too
    addString(IPP_TAG_OPERATION, IPP_TAG_NAME, KCUPS_REQUESTING_USER_NAME, cupsUser());
}

KIppRequest::~KIppRequest()
{
    Q_D(KIppRequest);
    delete d;
}

ipp_op_t KIppRequest::operation() const
{
    Q_D(const KIppRequest);
    return d->operation;
}

QString KIppRequest::resource() const
{
    Q_D(const KIppRequest);
    return d->resource;
}

QString KIppRequest::filename() const
{
    Q_D(const KIppRequest);
    return d->filename;
}

ipp_t *KIppRequest::sendIppRequest() const
{
    Q_D(const KIppRequest);

    ipp_t *request = ippNewRequest(d->operation);

    d->addRawRequestsToIpp(request);

    if (d->filename.isNull()) {
        return cupsDoRequest(CUPS_HTTP_DEFAULT, request, d->resource.toUtf8());
    } else {
        return cupsDoFileRequest(CUPS_HTTP_DEFAULT, request, d->resource.toUtf8(), d->filename.toUtf8());
    }
}

void KIppRequest::addString(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QString &value)
{
    Q_D(KIppRequest);

    d->addRequest(group, valueTag, name.toUtf8(), value);
}

void KIppRequest::addStringList(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QStringList &value)
{
    Q_D(KIppRequest);

    d->addRequest(group, valueTag, name.toUtf8(), value);
}

void KIppRequest::addInteger(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, int value)
{
    Q_D(KIppRequest);

    d->addRequest(group, valueTag, name.toUtf8(), value);
}

void KIppRequest::addBoolean(ipp_tag_t group, const QString &name, bool value)
{
    Q_D(KIppRequest);

    d->addRequest(group, IPP_TAG_ZERO, name.toUtf8(), value);
}

void KIppRequest::addVariantValues(const QVariantHash &values)
{
    QVariantHash::ConstIterator i = values.constBegin();
    while (i != values.constEnd()) {
        QString key = i.key();
        QVariant value = i.value();
        switch (value.type()) {
        case QVariant::Bool:
            if (key == QLatin1String(KCUPS_PRINTER_IS_ACCEPTING_JOBS)) {
                addBoolean(IPP_TAG_PRINTER, key, value.toBool());
            } else {
                addBoolean(IPP_TAG_OPERATION, key, value.toBool());
            }
            break;
        case QVariant::Int:
            if (key == QLatin1String(KCUPS_JOB_ID)) {
                addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, key, value.toInt());
            } else if (key == QLatin1String(KCUPS_PRINTER_STATE)) {
                addInteger(IPP_TAG_PRINTER, IPP_TAG_ENUM, key, value.toInt());
            } else {
                addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, key, value.toInt());
            }
            break;
        case QVariant::String:
            if (key == QLatin1String(KCUPS_DEVICE_URI)) {
                // device uri has a different TAG
                addString(IPP_TAG_PRINTER, IPP_TAG_URI, key, value.toString());
            } else if (key == QLatin1String(KCUPS_JOB_PRINTER_URI)) {
                addString(IPP_TAG_OPERATION, IPP_TAG_URI, key, value.toString());
            } else if (key == QLatin1String(KCUPS_PRINTER_OP_POLICY) ||
                       key == QLatin1String(KCUPS_PRINTER_ERROR_POLICY) ||
                       key == QLatin1String("ppd-name")) {
                // printer-op-policy has a different TAG
                addString(IPP_TAG_PRINTER, IPP_TAG_NAME, key, value.toString());
            } else if (key == QLatin1String(KCUPS_JOB_NAME)) {
                addString(IPP_TAG_OPERATION, IPP_TAG_NAME, key, value.toString());
            } else if (key == QLatin1String(KCUPS_WHICH_JOBS)) {
                addString(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, key, value.toString());
            } else {
                addString(IPP_TAG_PRINTER, IPP_TAG_TEXT, key, value.toString());
            }
            break;
        case QVariant::StringList:
            if (key == QLatin1String(KCUPS_MEMBER_URIS)) {
                addStringList(IPP_TAG_PRINTER, IPP_TAG_URI, key, value.toStringList());
            } else if (key == QLatin1String("notify-events")) {
                // Used for DBus notification, the values contains
                // what we want to watch
                addStringList(IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD, key, value.toStringList());
            } else {
                addStringList(IPP_TAG_PRINTER, IPP_TAG_NAME, key, value.toStringList());
            }
            break;
        case QVariant::UInt:
            addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, key, value.toInt());
            break;
        default:
            kWarning() << "type NOT recognized! This will be ignored:" << key << "values" << i.value();
        }
        ++i;
    }
}

void KIppRequest::addPrinterUri(const QString &printerName, bool isClass)
{
    QString uri = assembleUrif(printerName, isClass);
    addString(IPP_TAG_OPERATION, IPP_TAG_URI, KCUPS_PRINTER_URI, uri);
}

QString KIppRequest::assembleUrif(const QString &name, bool isClass)
{
    char  uri[HTTP_MAX_URI]; // printer URI

    QString destination;
    if (isClass) {
        destination = QLatin1String("/classes/") % name;
    } else {
        destination = QLatin1String("/printers/") % name;
    }

    httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", cupsUser(), "localhost",
                     ippPort(), destination.toUtf8());
    return uri;
}

KIppRequest &KIppRequest::operator =(const KIppRequest &other)
{
    Q_D(KIppRequest);
    if (this == &other)
        return *this;

    *d = *other.d_ptr;

    return *this;
}

void KIppRequestPrivate::addRequest(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QVariant &value)
{
    KCupsRawRequest request;
    request.group = group;
    request.valueTag = valueTag;
    request.name = name;
    request.value = value;

    rawRequests << request;
}

void KIppRequestPrivate::addRawRequestsToIpp(ipp_t *ipp) const
{
    // sort the values as CUPS requires it
    qSort(rawRequests.begin(), rawRequests.end(), rawRequestGroupLessThan);

    foreach (const KCupsRawRequest &request, rawRequests) {
        switch (request.value.type()) {
        case QVariant::Bool:
            ippAddBoolean(ipp,
                          request.group,
                          request.name.toUtf8(),
                          request.value.toBool());
            break;
        case QVariant::Int:
        case QVariant::UInt:
            ippAddInteger(ipp,
                          request.group,
                          request.valueTag,
                          request.name.toUtf8(),
                          request.value.toInt());
            break;
        case QVariant::String:
            ippAddString(ipp,
                         request.group,
                         request.valueTag,
                         request.name.toUtf8(),
                         "utf-8",
                         request.value.toString().toUtf8());
            break;
        case QVariant::StringList:
        {
            QStringList list = request.value.toStringList();
            const char **values = qStringListToCharPtrPtr(list);

            ippAddStrings(ipp,
                          request.group,
                          request.valueTag,
                          request.name.toUtf8(),
                          list.size(),
                          "utf-8",
                          values);

            // ippAddStrings deep copies everything so we can throw away the values.
            // the QBAList and content is auto discarded when going out of scope.
            delete [] values;
            break;
        }
        default:
            kWarning() << "type NOT recognized! This will be ignored:" << request.name << "values" << request.value;
        }
    }
}
