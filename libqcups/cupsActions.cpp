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

#include <KDebug>

bool QCups::cupsMoveJob(const char *name, int job_id, const char *dest_name)
{
    char  uri[HTTP_MAX_URI]; // Job/printer URI
    char  destUri[HTTP_MAX_URI]; // new printer URI
    ipp_t *request;

    // check the input data
    if (job_id < -1 || (!name && !dest_name && job_id == 0)) {
        qWarning() << "Internal error, invalid input data" << job_id << name << dest_name;
        return false;
    }

    // Create a new CUPS_MOVE_JOB request
    // where we need:
    // * printer-uri
    // * job-printer-uri
    // * job-id
    // * requesting-user-name
    request = ippNewRequest(CUPS_MOVE_JOB);

    httpAssembleURIf(HTTP_URI_CODING_ALL, destUri, sizeof(destUri), "ipp", NULL,
                    "localhost", ippPort(), "/printers/%s", dest_name);

    httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
                    "localhost", ippPort(), "/printers/%s", name);

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                 NULL, uri);
    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "job-id",
                  job_id);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "job-printer-uri",
                 NULL, destUri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
                 NULL, cupsUser());

    // do the request deleting the response
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/jobs/"));

    return !cupsLastError();
}

bool QCups::cupsHoldReleaseJob(const char *name, int job_id, bool hold)
{
    char  uri[HTTP_MAX_URI]; // Job/printer URI
    ipp_t *request;

    // check the input data
    if (job_id < -1 || (!name && job_id == 0)) {
        qWarning() << "Internal error, invalid input data" << job_id << name;
        return false;
    }

    // Create a new CUPS_MOVE_JOB request
    // where we need:
    // * printer-uri
    // * job-id
    // * requesting-user-name
    if (hold) {
        request = ippNewRequest(IPP_HOLD_JOB);
    } else {
        request = ippNewRequest(IPP_RELEASE_JOB);
    }

    httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
                    "localhost", ippPort(), "/printers/%s", name);

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                 NULL, uri);
    ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER, "job-id",
                  job_id);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
                 NULL, cupsUser());

    // do the request deleting the response
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/jobs/"));

    return !cupsLastError();
}

bool QCups::cupsPauseResumePrinter(const char *name, bool pause)
{
    char  uri[HTTP_MAX_URI]; // printer URI
    ipp_t *request;

    // check the input data
    if (!name) {
        qWarning() << "Internal error, invalid input data" << name;
        return false;
    }

    // Create a new request
    // where we need:
    // * printer-uri
    // * requesting-user-name
    if (pause) {
        request = ippNewRequest(IPP_PAUSE_PRINTER);
    } else {
        request = ippNewRequest(IPP_RESUME_PRINTER);
    }

    httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
                    "localhost", ippPort(), "/printers/%s", name);

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                 NULL, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
                 NULL, cupsUser());

    // do the request deleting the response
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/"));

    return !cupsLastError();
}

bool QCups::cupsSetDefaultPrinter(const char *name)
{
    char  uri[HTTP_MAX_URI]; // printer URI
    ipp_t *request;

    // check the input data
    if (!name) {
        qWarning() << "Internal error, invalid input data" << name;
        return false;
    }

    // Create a new request
    // where we need:
    // * printer-uri
    // * requesting-user-name
    request = ippNewRequest(CUPS_SET_DEFAULT);

    httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
                    "localhost", ippPort(), "/printers/%s", name);

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                 NULL, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
                 NULL, cupsUser());

    // do the request deleting the response
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/"));

    return !cupsLastError();
}

bool QCups::cupsDeletePrinter(const char *name)
{
    char  uri[HTTP_MAX_URI]; // printer URI
    ipp_t *request;

    // check the input data
    if (!name) {
        qWarning() << "Internal error, invalid input data" << name;
        return false;
    }

    // Create a new request
    // where we need:
    // * printer-uri
    // * requesting-user-name
    request = ippNewRequest(CUPS_DELETE_PRINTER);

    httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
                    "localhost", ippPort(), "/printers/%s", name);

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                 NULL, uri);
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
                 NULL, cupsUser());

    // do the request deleting the response
    ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/admin/"));

    return !cupsLastError();
}
