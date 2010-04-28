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
#include "QCups.h"
#include <cups/adminutil.h>

#include <QMetaObject>
#include <QGenericArgument>
#include <QStringList>
#include <KDebug>
#include <KLocale>
#include <QEventLoop>
#include <QMutexLocker>
#include <QPointer>
// #include <KPasswordDialog>

#define CUPS_DATADIR    "/usr/share/cups"

using namespace QCups;

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<bool>)

ReturnArguments cupsParseIPPVars(ipp_t *response, bool needDestName);

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
                     kDebug() << name << isClass << operation << uri;
    return request;
}
// QWaitCondition returnCondition;
// QMutex mutex;
static uint password_retries = 0;
const char * thread_password_cb(const char *prompt, http_t *http, const char *method, const char *resource, void *user_data)
{
    Q_UNUSED(prompt)
    Q_UNUSED(http)
    Q_UNUSED(method)
    Q_UNUSED(resource)

    kDebug() << QThread::currentThreadId()
             << "-----------thread_password_cb------"<< "password_retries" << password_retries;

    if (password_retries == 3) {
        // cancel the authentication
        cupsSetUser(NULL);
        return NULL;
    }

    bool showErrorMessage = false;
    if (password_retries) {
        showErrorMessage = true;
    }

    CupsThreadRequest *thread = static_cast<CupsThreadRequest*>(user_data);
    QEventLoop *loop = new QEventLoop;
    QMetaObject::invokeMethod(thread->parent(),
                              "showPasswordDlg",
                              Qt::QueuedConnection,
                              Q_ARG(QEventLoop*, loop),
                              Q_ARG(QString, QString::fromUtf8(cupsUser())),
                              Q_ARG(bool, showErrorMessage));
//     emit thread->req->showPasswordDlg(loop, QString::fromUtf8(cupsUser()), showErrorMessage);
    loop->exec();
    kDebug() << "END OF THREAD EXEC";

    kDebug() << "~~~~~~RELEASED";

    QObject *response = loop;
    if (response->property("canceled").toBool()) {
        // the dialog was canceled
        password_retries = 3;
        cupsSetUser(NULL);
        return NULL;
    } else {
        QString username = response->property("username").toString();
        QString password = response->property("password").toString();
        cupsSetUser(username.toUtf8());
        return password.toUtf8();
    }
}

CupsThreadRequest::CupsThreadRequest(QObject *parent)
 : QThread(parent), req(0)
{
    qRegisterMetaType<ipp_op_e>("ipp_op_e");
    qRegisterMetaType<bool*>("bool*");
    qRegisterMetaType<Arguments>("Arguments");
    qRegisterMetaType<QEventLoop*>("QEventLoop*");
    qRegisterMetaType<Result*>("Result*");
}

CupsThreadRequest::~CupsThreadRequest()
{
    req->deleteLater();
}

void CupsThreadRequest::run()
{
    kDebug() << QThread::currentThreadId();
    cupsSetPasswordCB2(thread_password_cb, this);
    req = new Request();
    exec();
}

bool Request::retry()
{
//     http_t     *http = CUPS_HTTP_DEFAULT;
    kDebug() << "cupsLastErrorString()" << cupsLastErrorString() << cupsLastError() << IPP_FORBIDDEN;
//     kDebug() << http->digest_tries;
    if (cupsLastError() == IPP_FORBIDDEN ||
        cupsLastError() == IPP_NOT_AUTHORIZED ||
        cupsLastError() == IPP_NOT_AUTHENTICATED) {
        if (password_retries == 0) {
            // try to authenticate as the root user
            cupsSetUser("root");
        } else if (password_retries >= 3) {
            // the authentication failed 3 times
            // OR the dialog was canceld (-1)
            // reset to 0 and quit the do-while loop
            password_retries = 0;
            return false;
        }

        // force authentication
        kDebug() << "cupsDoAuthentication" << password_retries;
        if (cupsDoAuthentication(CUPS_HTTP_DEFAULT, "POST", "/") == -1) {
        }
        password_retries++;
        // tries to do the action again
        // sometimes just trying to be root works
        return true;
    }
    // the action was not forbidden
    return false;
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

ReturnArguments cupsParseIPPVars(ipp_t *response, bool needDestName)
{
    ipp_attribute_t *attr;
    ReturnArguments ret;

    for (attr = response->attrs; attr != NULL; attr = attr->next) {
          /*
           * Skip leading attributes until we hit a printer...
           */
          while (attr != NULL && attr->group_tag != IPP_TAG_PRINTER) {
              attr = attr->next;
          }

          if (attr == NULL) {
              break;
          }

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
                  attr->value_tag != IPP_TAG_LANGUAGE &&
                  attr->value_tag != IPP_TAG_NAME &&
                  attr->value_tag != IPP_TAG_NAMELANG &&
                  attr->value_tag != IPP_TAG_KEYWORD &&
                  attr->value_tag != IPP_TAG_RANGE &&
                  attr->value_tag != IPP_TAG_URI)
                  continue;

//               if (!strcmp(attr->name, "printer-name") ||
//                   !strcmp(attr->name, "auth-info-required") ||
//                   !strcmp(attr->name, "device-uri") ||
//                   !strcmp(attr->name, "marker-change-time") ||
//                   !strcmp(attr->name, "marker-colors") ||
//                   !strcmp(attr->name, "marker-high-levels") ||
//                   !strcmp(attr->name, "marker-levels") ||
//                   !strcmp(attr->name, "marker-low-levels") ||
//                   !strcmp(attr->name, "marker-message") ||
//                   !strcmp(attr->name, "marker-names") ||
//                   !strcmp(attr->name, "marker-types") ||
//                   !strcmp(attr->name, "printer-commands") ||
//                   !strcmp(attr->name, "printer-info") ||
//                   !strcmp(attr->name, "printer-is-shared") ||
//                   !strcmp(attr->name, "printer-make-and-model") ||
//                   !strcmp(attr->name, "printer-state") ||
//                   !strcmp(attr->name, "printer-state-change-time") ||
//                   !strcmp(attr->name, "printer-type") ||
//                   !strcmp(attr->name, "printer-is-accepting-jobs") ||
//                   !strcmp(attr->name, "printer-location") ||
//                   !strcmp(attr->name, "printer-state-reasons") ||
//                   !strcmp(attr->name, "printer-state-message") ||
//                   !strcmp(attr->name, "printer-uri-supported") ||
//                   !strcmp(attr->name, "ppd-name") ||
//                   !strcmp(attr->name, "ppd-make") ||
//                   !strcmp(attr->name, "ppd-make-and-model") ||
//                   !strcmp(attr->name, "ppd-natural-language"))
//               {
                  /*
                  * Add a printer description attribute...
                  */
                  destAttributes[QString::fromUtf8(attr->name)] = cupsMakeVariant(attr);
//                     QString::fromUtf8(attr->values[0].string.text);
//                     num_options = cupsAddOption(attr->name,
//                                                 cups_make_string(attr, value,
//                                                                 sizeof(value)),
//                                                 num_options, &options);
//               }

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
          if (needDestName && destAttributes["printer-name"].toString().isEmpty())
          {
//                 cupsFreeOptions(num_options, options);

              if (attr == NULL)
                  break;
              else
                  continue;
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
      return ret;
}

void Request::request(Result *result, ipp_op_e operation, const QString &resource,  Arguments reqValues)
{
    kDebug() << "BEGIN" << operation << resource << QThread::currentThreadId();
    password_retries = 0;
    do {
        ipp_t *request;
        ipp_t *response;
        bool isClass = false;
        bool needDestName = false;
        const char *name = NULL;
        const char *filename = NULL;
        QHash<QString, QVariant> values = reqValues;

        if (values.contains("printer-is-class")) {
            isClass = values.take("printer-is-class").toBool();
        }
        if (values.contains("need-dest-name")) {
            needDestName = values.take("need-dest-name").toBool();
        }
        if (values.contains("printer-name")) {
            name = qstrdup(values.take("printer-name").toString().toUtf8());
        }

        if (values.contains("filename")) {
            filename = values.take("filename").toString().toUtf8();
        }

        // Lets create the request
        if (name) {
            request = ippNewDefaultRequest(name, isClass, operation);
        } else {
            request = ippNewRequest(operation);
        }

        QHash<QString, QVariant>::const_iterator i = values.constBegin();
        while (i != values.constEnd()) {
            switch (i.value().type()) {
            case QVariant::Bool:
                ippAddBoolean(request, IPP_TAG_OPERATION,
                            i.key().toUtf8(), i.value().toBool());
                break;
            case QVariant::Int:
                ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_ENUM,
                            i.key().toUtf8(), i.value().toInt());
                break;
            case QVariant::String:
                if (i.key() == "device-uri") {
                    // device uri has a different TAG
                    ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                                i.key().toUtf8(), "utf-8",
                                i.value().toString().toUtf8());
                } else if (i.key() == "printer-op-policy" ||
                        i.key() == "printer-error-policy" ||
                        i.key() == "ppd-name") {
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
                                            "member-uris", list.size(), "utf-8", NULL);
                    } else if (i.key() == "requested-attributes") {
                        attr = ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                                            "requested-attributes", list.size(), "utf-8", NULL);
                    } else {
                        attr = ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                                            i.key().toUtf8(), list.size(), "utf-8", NULL);
                    }
                    // Dump all the list values
                    for (int i = 0; i < list.size(); i++) {
                        attr->values[i].string.text = qstrdup(list.at(i).toUtf8());
                    }
                }
                break;
            default:
                kWarning() << "type NOT recognized! This will be ignored:" << i.key() << "values" << i.value();
            }
            ++i;
        }

        // Do the request
        // do the request deleting the response
        if (filename) {
            response = cupsDoFileRequest(CUPS_HTTP_DEFAULT, request, resource.toUtf8(), filename);
        } else {
            response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, resource.toUtf8());
        }

        if (response != NULL) {
    //         kDebug() << "response" << response;
            // TODO add an element to NeedDestName
            result->setResult(cupsParseIPPVars(response, needDestName));

    //         kDebug() << "m_returnedValues " << m_returnedValues;
            ippDelete(response);
        }
        result->setLastError(cupsLastError());
        result->setLastErrorString(QString::fromUtf8(cupsLastErrorString()));
        kDebug() << QThread::currentThreadId();
    } while (retry());
    kDebug() << "END" << QThread::currentThreadId();
    emit finished();
}

ipp_status_t QCups::cupsAddModifyClassOrPrinter(const char *name, bool is_class, const QHash<QString, QVariant> values, const char *filename)
{
    ipp_t *request;

    // check the input data
    if (!name) {
        qWarning() << "Internal error, invalid input data" << name;
        return IPP_OK;
    }

    if (is_class && values.contains("member-uris")) {
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
                    i.key() == "printer-error-policy" ||
                    i.key() == "ppd-name") {
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
            break;
        default:
            kWarning() << "type NOT recognized! This will be ignored:" << i.key() << "values" << i.value();
        }
        ++i;
    }

ipp_status_t ret;
    // do the request deleting the response
    if (filename) {
        ippDelete(cupsDoFileRequest(CUPS_HTTP_DEFAULT, request, "/admin/", filename));
//         ret = cupsLastError();
    } else {
        kDebug();
//         CupsThreadRequest *thread = new CupsThreadRequest(request, "/admin/");
//         thread->execute();
//         ret = thread->lastError();
        kDebug();
//         delete thread;
//         ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/"));
    }

    return ret;
}

ipp_status_t QCups::cupsMoveJob(const char *name, int job_id, const char *dest_name)
{
    char  destUri[HTTP_MAX_URI]; // new printer URI
    ipp_t *request;

    // check the input data
    if (job_id < -1 || (!name && !dest_name && job_id == 0)) {
        qWarning() << "Internal error, invalid input data" << job_id << name << dest_name;
        return IPP_OK;
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

    return cupsLastError();
}

ipp_status_t QCups::cupsHoldReleaseJob(const char *name, int job_id, bool hold)
{
    ipp_t *request;

    // check the input data
    if (job_id < -1 || (!name && job_id == 0)) {
        qWarning() << "Internal error, invalid input data" << job_id << name;
        return IPP_OK;
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

    return cupsLastError();
}

ipp_status_t QCups::cupsPrintTestPage(const char *name, bool is_class)      /* I - Destination printer/class */
{
    ipp_t         *request;               /* IPP request */
    char          resource[1024],         /* POST resource path */
                  filename[1024];         /* Test page filename */
    const char    *datadir;               /* CUPS_DATADIR env var */

    /*
    * Locate the test page file...
    */
    if ((datadir = getenv("CUPS_DATADIR")) == NULL)
        datadir = CUPS_DATADIR;

    snprintf(filename, sizeof(filename), "%s/data/testprint", datadir);

    /*
    * Point to the printer/class...
    */
    snprintf(resource, sizeof(resource),
             is_class ? "/classes/%s" : "/printers/%s", name);

    // Build a new default request
    request = ippNewDefaultRequest(name, is_class, IPP_PRINT_JOB);


    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "job-name",
                 "utf-8", i18n("Test Page").toUtf8());

    // do the request deleting the response
    ippDelete(cupsDoFileRequest(CUPS_HTTP_DEFAULT, request, resource, filename));

    return cupsLastError();
}

bool QCups::cupsPrintCommand(const char *name,       /* I - Destination printer */
                             const char *command,    /* I - Command to send */
                             const char *title)      /* I - Page/job title */
{
    int           job_id;                 /* Command file job */
    char          command_file[1024];     /* Command "file" */
    http_status_t status;                 /* Document status */
    cups_option_t hold_option;            /* job-hold-until option */


    /*
     * Create the CUPS command file...
     */
    snprintf(command_file, sizeof(command_file), "#CUPS-COMMAND\n%s\n", command);

    /*
     * Send the command file job...
     */
    hold_option.name  = const_cast<char*>("job-hold-until");
    hold_option.value = const_cast<char*>("no-hold");

    if ((job_id = cupsCreateJob(CUPS_HTTP_DEFAULT, name, title,
                                1, &hold_option)) < 1)
    {
        qWarning() << "Unable to send command to printer driver!";
        return false;
    }

    status = cupsStartDocument(CUPS_HTTP_DEFAULT, name, job_id, NULL, CUPS_FORMAT_COMMAND, 1);
    if (status == HTTP_CONTINUE) {
        status = cupsWriteRequestData(CUPS_HTTP_DEFAULT, command_file,
                                      strlen(command_file));
    }

    if (status == HTTP_CONTINUE) {
        cupsFinishDocument(CUPS_HTTP_DEFAULT, name);
    }

    if (cupsLastError() >= IPP_REDIRECTION_OTHER_SITE)
    {
        qWarning() << "Unable to send command to printer driver!";

        cupsCancelJob(name, job_id);
        return false;
    }
    return true;
}

QHash<QString, QString>QCups::cupsAdminGetServerSettings()
{
    int num_settings;
    cups_option_t *settings;
    QHash<QString, QString> ret;
    cupsAdminGetServerSettings(CUPS_HTTP_DEFAULT, &num_settings, &settings);
    for (int i = 0; i < num_settings; i++) {
        QString name = QString::fromUtf8(settings[i].name);
        QString value = QString::fromUtf8(settings[i].value);
        ret[name] = value;
    }
    cupsFreeOptions(num_settings, settings);

    return ret;
}

ipp_status_t QCups::cupsAdminSetServerSettings(const QHash<QString, QString> &userValues)
{
    bool ret = false;
    int num_settings = 0;
    cups_option_t *settings;

    if (userValues.contains("_remote_admin")) {
        num_settings = cupsAddOption(CUPS_SERVER_REMOTE_ADMIN,
                                     userValues["_remote_admin"].toUtf8(), num_settings, &settings);
    }
    if (userValues.contains("_remote_any")) {
        num_settings = cupsAddOption(CUPS_SERVER_REMOTE_ANY,
                                     userValues["_remote_any"].toUtf8(), num_settings, &settings);
    }
    if (userValues.contains("_remote_printers")) {
        num_settings = cupsAddOption(CUPS_SERVER_REMOTE_PRINTERS,
                                     userValues["_remote_printers"].toUtf8(), num_settings, &settings);
    }
    if (userValues.contains("_share_printers")) {
        num_settings = cupsAddOption(CUPS_SERVER_SHARE_PRINTERS,
                                     userValues["_share_printers"].toUtf8(), num_settings, &settings);
    }
    if (userValues.contains("_user_cancel_any")) {
        num_settings = cupsAddOption(CUPS_SERVER_USER_CANCEL_ANY,
                                     userValues["_user_cancel_any"].toUtf8(), num_settings, &settings);
    }

    ret = cupsAdminSetServerSettings(CUPS_HTTP_DEFAULT, num_settings, settings);
    cupsFreeOptions(num_settings, settings);

    return cupsLastError();
}
