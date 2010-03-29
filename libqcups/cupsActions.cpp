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

#include "cupsActions.h"
#include <cups/cups.h>

#include <QStringList>
#include <KDebug>

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<bool>)

// Don't forget to delete the request
ipp_t * ippNewDefaultRequest(const char *name, bool isClass, ipp_op_t operation)
{
    char  uri[HTTP_MAX_URI]; // printer URI
    ipp_t *request;

    // Create a new request
    // where we need:
    // * printer-uri
    // * requesting-user-name
    request = ippNewRequest(operation);
    httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", "utf-8",
                     "localhost", ippPort(), isClass ? "/classes/%s" : "/printers/%s",
                     name);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                 "utf-8", uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
                 "utf-8", cupsUser());
    return request;
}

QHash<QString, QVariant> QCups::cupsGetAttributes(const char *name, bool is_class, const QStringList &requestedAttr)
{
    ipp_t *request, *response;
    ipp_attribute_t *attr;
    QHash<QString, QVariant> responseSL;
    char **attributes;

    // check the input data
    if (!name || requestedAttr.size() == 0) {
        qWarning() << "Internal error, invalid input data" << name;
        return responseSL;
    }

    attributes = new char*[requestedAttr.size()];
    for (int i = 0; i < requestedAttr.size(); i ++) {
        attributes[i] = qstrdup(requestedAttr.at(i).toUtf8());
    }

    request = ippNewDefaultRequest(name, is_class, IPP_GET_PRINTER_ATTRIBUTES);

    ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                  "requested-attributes", requestedAttr.size(),
                  "utf-8", attributes);

    // do the request
    if ((response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/")) != NULL) {

        for (attr = response->attrs; attr != NULL; attr = attr->next) {
            if (attr->value_tag == IPP_TAG_INTEGER || attr->value_tag == IPP_TAG_ENUM) {
                if (attr->num_values == 1) {
                    responseSL[QString::fromUtf8(attr->name)] = attr->values[0].integer;
                } else {
                    QList<int> values;
                    for (int i = 0; i < attr->num_values; i++) {
                        values << attr->values[i].integer;
                        printf ("Attribute: %s == %d == %d\n", attr->name, attr->num_values, attr->values[i].integer);
                    }
                    responseSL[QString::fromUtf8(attr->name)] = QVariant::fromValue(values);
                }
            } else if (attr->value_tag == IPP_TAG_BOOLEAN ) {
                QList<bool> values;
                for (int i = 0; i < attr->num_values; i++) {
                    values << attr->values[i].integer;
                    printf ("Attribute: %s == %d == %d\n", attr->name, attr->num_values, attr->values[i].boolean);
                }
                responseSL[QString::fromUtf8(attr->name)] = QVariant::fromValue(values);
            } else {
                QStringList values;
                for (int i = 0; i < attr->num_values; i++) {
                    values << QString::fromUtf8(attr->values[i].string.text);
                    printf ("Attribute: %s == %d == %s\n", attr->name, attr->num_values, attr->values[i].string.text);
                }
                responseSL[QString::fromUtf8(attr->name)] = values;
            }
        }

        ippDelete(response);
    }

    for (int i = 0; i < requestedAttr.size(); i ++) {
        delete attributes[i];
    }
    delete [] attributes;

    return responseSL;
}

bool QCups::cupsAddModifyClassOrPrinter(const char *name, bool is_class, const QHash<QString, QVariant> values)
{
    ipp_t *request;

    // check the input data
    if (!name) {
        qWarning() << "Internal error, invalid input data" << name;
        return false;
    }

    if (is_class && values.contains("member-uris")) {
      kDebug();
        request = ippNewDefaultRequest(name, is_class, CUPS_ADD_CLASS);
    } else {
        request = ippNewDefaultRequest(name, is_class,
                                       is_class ? CUPS_ADD_MODIFY_CLASS :
                                                  CUPS_ADD_MODIFY_PRINTER);
    }

    QHash<QString, QVariant>::const_iterator i = values.constBegin();
    while (i != values.constEnd()) {
        switch (i.value().type()) {
        case QVariant::Bool:
            ippAddBoolean(request, IPP_TAG_OPERATION, i.key().toUtf8(),
                          i.value().toBool());
            break;
        case QVariant::String:
            if (i.key() == "device-uri") {
                // device uri has a different TAG
                ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                             i.key().toUtf8(), "utf-8",
                             i.value().toString().toUtf8());
            } else if (i.key() == "printer-op-policy" ||
                       i.key() == "printer-error-policy") {
                // printer-op-policy has a different TAG
                ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                             i.key().toUtf8(), "utf-8",
                             i.value().toString().toUtf8());
            } else {
                ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT,
                             i.key().toUtf8(), "utf-8",
                             i.value().toString().toUtf8());
            }
            break;
        case QVariant::StringList:
            {
                ipp_attribute_t *attr;
                QStringList list = i.value().value<QStringList>();
                if (i.key() == "member-uris") {
                    attr = ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                                         i.key().toUtf8(), list.size(), NULL, NULL);
                } else {
                    attr = ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                                         i.key().toUtf8(), list.size(), NULL, NULL);
                }
                // Dump all the list values
                for (int i = 0; i < list.size(); i++) {
                    attr->values[i].string.text = qstrdup(list.at(i).toUtf8());
                }
            }
        default:
            kWarning() << "type NOT recognized! This will be ignored:" << i.key() << "values" << i.value();
        }
        ++i;
    }

    // do the request deleting the response
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/"));

    return !cupsLastError();
}

bool QCups::cupsMoveJob(const char *name, int job_id, const char *dest_name)
{
    char  destUri[HTTP_MAX_URI]; // new printer URI
    ipp_t *request;

    // check the input data
    if (job_id < -1 || (!name && !dest_name && job_id == 0)) {
        qWarning() << "Internal error, invalid input data" << job_id << name << dest_name;
        return false;
    }

    // Create a new CUPS_MOVE_JOB request
    // where we need:
    // * job-printer-uri
    // * job-id
    // TODO add class
    request = ippNewDefaultRequest(name, false, CUPS_MOVE_JOB);

    httpAssembleURIf(HTTP_URI_CODING_ALL, destUri, sizeof(destUri), "ipp", "utf-8",
                    "localhost", ippPort(), "/printers/%s", dest_name);

    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "job-id",
                  job_id);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-printer-uri",
                 "utf-8", destUri);

    // do the request deleting the response
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/jobs/"));

    return !cupsLastError();
}

bool QCups::cupsHoldReleaseJob(const char *name, int job_id, bool hold)
{
    ipp_t *request;

    // check the input data
    if (job_id < -1 || (!name && job_id == 0)) {
        qWarning() << "Internal error, invalid input data" << job_id << name;
        return false;
    }

    // Create a new CUPS_MOVE_JOB request
    // where we need:
    // * job-id
    // TODO add class
    if (hold) {
        request = ippNewDefaultRequest(name, false, IPP_HOLD_JOB);
    } else {
        request = ippNewDefaultRequest(name, false, IPP_RELEASE_JOB);
    }

    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "job-id",
                  job_id);

    // do the request deleting the response
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/jobs/"));

    return !cupsLastError();
}

bool QCups::cupsPauseResumePrinter(const char *name, bool pause)
{
    ipp_t *request;

    // check the input data
    if (!name) {
        qWarning() << "Internal error, invalid input data" << name;
        return false;
    }

    // TODO add class
    if (pause) {
        request = ippNewDefaultRequest(name, false, IPP_PAUSE_PRINTER);
    } else {
        request = ippNewDefaultRequest(name, false, IPP_RESUME_PRINTER);
    }

    // do the request deleting the response
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/"));

    return !cupsLastError();
}

bool QCups::cupsSetDefaultPrinter(const char *name)
{
    ipp_t *request;

    // check the input data
    if (!name) {
        qWarning() << "Internal error, invalid input data" << name;
        return false;
    }

    request = ippNewDefaultRequest(name, false, CUPS_SET_DEFAULT);

    // do the request deleting the response
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/"));

    return !cupsLastError();
}

bool QCups::cupsDeletePrinter(const char *name)
{
    ipp_t *request;

    // check the input data
    if (!name) {
        qWarning() << "Internal error, invalid input data" << name;
        return false;
    }

    // TODO add class
    request = ippNewDefaultRequest(name, false, CUPS_DELETE_PRINTER);

    // do the request deleting the response
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/"));

    return !cupsLastError();
}
