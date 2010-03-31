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
                    }
                    responseSL[QString::fromUtf8(attr->name)] = QVariant::fromValue(values);
                }
            } else if (attr->value_tag == IPP_TAG_BOOLEAN ) {
                QList<bool> values;
                for (int i = 0; i < attr->num_values; i++) {
                    values << attr->values[i].integer;
                }
                responseSL[QString::fromUtf8(attr->name)] = QVariant::fromValue(values);
            } else {
                if (attr->num_values == 1) {
                    responseSL[QString::fromUtf8(attr->name)] = QString::fromUtf8(attr->values[0].string.text);
                } else {
                    QStringList values;
                    for (int i = 0; i < attr->num_values; i++) {
                        values << QString::fromUtf8(attr->values[i].string.text);
                    }
                    responseSL[QString::fromUtf8(attr->name)] = values;
                }
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

static QVariant cupsMakeVariant(ipp_attribute_t *attr)
{
    if (attr->num_values == 1 &&
        attr->value_tag != IPP_TAG_INTEGER &&
        attr->value_tag != IPP_TAG_ENUM &&
        attr->value_tag != IPP_TAG_BOOLEAN &&
        attr->value_tag != IPP_TAG_RANGE) {
        return QString::fromUtf8(attr->values[0].string.text);
    }

    if (attr->value_tag == IPP_TAG_INTEGER || attr->value_tag == IPP_TAG_ENUM) {
        if (attr->num_values == 1) {
            return attr->values[0].integer;
        } else {
            QList<int> values;
            for (int i = 0; i < attr->num_values; i++) {
                values << attr->values[i].integer;
            }
            return QVariant::fromValue(values);
        }
    } else if (attr->value_tag == IPP_TAG_BOOLEAN ) {
        if (attr->num_values == 1) {
            return static_cast<bool>(attr->values[0].integer);
        } else {
            QList<bool> values;
            for (int i = 0; i < attr->num_values; i++) {
                values << static_cast<bool>(attr->values[i].integer);
            }
            return QVariant::fromValue(values);
        }
    } else if (attr->value_tag == IPP_TAG_RANGE) {
        QVariantList values;
        for (int i = 0; i < attr->num_values; i++) {
            values << attr->values[i].range.lower;
            values << attr->values[i].range.upper;
        }
        return values;
    } else {
        QStringList values;
        for (int i = 0; i < attr->num_values; i++) {
            values << QString::fromUtf8(attr->values[i].string.text);
        }
        return values;
    }
}

QList<QHash<QString, QVariant> > QCups::cupsGetDests(int mask, const QStringList &requestedAttr)
{
    ipp_attribute_t *attr;
    ipp_t *request;
    ipp_t *response;
    char **attributes;
    QList<QHash<QString, QVariant> > ret;

    QString defaultDest = QString::fromUtf8(cupsGetDefault());

    request = ippNewRequest(CUPS_GET_PRINTERS);

    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_ENUM, "printer-type",
                  CUPS_PRINTER_LOCAL);
    if (mask >= 0){
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_ENUM, "printer-type-mask",
                      mask);
    }

    if (!requestedAttr.isEmpty()){
        attributes = new char*[requestedAttr.size()];
        for (int i = 0; i < requestedAttr.size(); i ++) {
            attributes[i] = qstrdup(requestedAttr.at(i).toUtf8());
        }
        ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                    "requested-attributes", requestedAttr.size(),
                    "utf-8", attributes);
    }

    if ((response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/")) != NULL) {
        for (attr = response->attrs; attr != NULL; attr = attr->next) {
            /*
            * Skip leading attributes until we hit a printer...
            */

            while (attr != NULL && attr->group_tag != IPP_TAG_PRINTER)
                attr = attr->next;

            if (attr == NULL)
                break;
                /*
            * Pull the needed attributes from this printer...
            */

            QHash<QString, QVariant> destAttributes;

            for (; attr && attr->group_tag == IPP_TAG_PRINTER; attr = attr->next)
            {
                if (attr->value_tag != IPP_TAG_INTEGER &&
                    attr->value_tag != IPP_TAG_ENUM &&
                    attr->value_tag != IPP_TAG_BOOLEAN &&
                    attr->value_tag != IPP_TAG_TEXT &&
                    attr->value_tag != IPP_TAG_TEXTLANG &&
                    attr->value_tag != IPP_TAG_NAME &&
                    attr->value_tag != IPP_TAG_NAMELANG &&
                    attr->value_tag != IPP_TAG_KEYWORD &&
                    attr->value_tag != IPP_TAG_RANGE &&
                    attr->value_tag != IPP_TAG_URI)
                    continue;

                if (!strcmp(attr->name, "printer-name") ||
                    !strcmp(attr->name, "auth-info-required") ||
                    !strcmp(attr->name, "device-uri") ||
                    !strcmp(attr->name, "marker-change-time") ||
                    !strcmp(attr->name, "marker-colors") ||
                    !strcmp(attr->name, "marker-high-levels") ||
                    !strcmp(attr->name, "marker-levels") ||
                    !strcmp(attr->name, "marker-low-levels") ||
                    !strcmp(attr->name, "marker-message") ||
                    !strcmp(attr->name, "marker-names") ||
                    !strcmp(attr->name, "marker-types") ||
                    !strcmp(attr->name, "printer-commands") ||
                    !strcmp(attr->name, "printer-info") ||
                    !strcmp(attr->name, "printer-is-shared") ||
                    !strcmp(attr->name, "printer-make-and-model") ||
                    !strcmp(attr->name, "printer-state") ||
                    !strcmp(attr->name, "printer-state-change-time") ||
                    !strcmp(attr->name, "printer-type") ||
                    !strcmp(attr->name, "printer-is-accepting-jobs") ||
                    !strcmp(attr->name, "printer-location") ||
                    !strcmp(attr->name, "printer-state-reasons") ||
                    !strcmp(attr->name, "printer-state-message") ||
                    !strcmp(attr->name, "printer-uri-supported"))
                {
                    /*
                    * Add a printer description attribute...
                    */
                    destAttributes[QString::fromUtf8(attr->name)] = cupsMakeVariant(attr);
//                     QString::fromUtf8(attr->values[0].string.text);
//                     num_options = cupsAddOption(attr->name,
//                                                 cups_make_string(attr, value,
//                                                                 sizeof(value)),
//                                                 num_options, &options);
                }

//                 else if (!strcmp(attr->name, "printer-name") &&
//                         attr->value_tag == IPP_TAG_NAME)
//                     printer_name = QString::fromUtf8(attr->values[0].string.text);
//                 else if (strncmp(attr->name, "notify-", 7) &&
//                         (attr->value_tag == IPP_TAG_BOOLEAN ||
//                         attr->value_tag == IPP_TAG_ENUM ||
//                         attr->value_tag == IPP_TAG_INTEGER ||
//                         attr->value_tag == IPP_TAG_KEYWORD ||
//                         attr->value_tag == IPP_TAG_NAME ||
//                         attr->value_tag == IPP_TAG_RANGE) &&
//                         (ptr = strstr(attr->name, "-default")) != NULL)
//                 {
//                     /*
//                     * Add a default option...
//                     */
//
//                     strlcpy(optname, attr->name, sizeof(optname));
//                     optname[ptr - attr->name] = '\0';
//
//                     if (strcasecmp(optname, "media") ||
//                         !cupsGetOption("media", num_options, options))
//                         num_options = cupsAddOption(optname,
//                                                     cups_make_string(attr, value,
//                                                                     sizeof(value)),
//                                                     num_options, &options);
//                 }
            }
            /*
            * See if we have everything needed...
            */

            if (destAttributes["printer-name"].toString().isEmpty())
            {
//                 cupsFreeOptions(num_options, options);

                if (attr == NULL)
                    break;
                else
                    continue;
            }

            if (requestedAttr.contains("printer-is-default") &&
                defaultDest == destAttributes["printer-name"].toString()) {
                destAttributes["printer-is-default"] = true;
            }

            ret << destAttributes;
//             if ((dest = cups_add_dest(printer_name, NULL, &num_dests, dests)) != NULL)
//             {
//                 dest->num_options = num_options;
//                 dest->options     = options;
//             }
//             else
//                 cupsFreeOptions(num_options, options);
//
            if (attr == NULL)
                break;

        }

        ippDelete(response);
    }

    return ret;
}

bool QCups::cupsAddModifyClassOrPrinter(const char *name, bool is_class, const QHash<QString, QVariant> values, const char *filename)
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
    if (filename) {
        ippDelete(cupsDoFileRequest(CUPS_HTTP_DEFAULT, request, "/admin/", filename));
    } else {
        ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/"));
    }

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
