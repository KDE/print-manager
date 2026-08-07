/*
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <kcups_export.h>

#ifdef LIBCUPS_VERSION_2
#include <cups/adminutil.h>
#include <cups/cups.h>
#include <cups/ppd.h>

#define KCUPS_PRINTER_COLOR CUPS_PRINTER_COLOR
#define KCUPS_PRINTER_SCANNER CUPS_PRINTER_SCANNER
#define KCUPS_PRINTER_REMOTE CUPS_PRINTER_REMOTE
#define KCUPS_PRINTER_LOCAL CUPS_PRINTER_LOCAL
#define KCUPS_PRINTER_CLASS CUPS_PRINTER_CLASS
#define KCUPS_PRINTER_DEFAULT CUPS_PRINTER_DEFAULT
#define KCUPS_PRINTER_DISCOVERED CUPS_PRINTER_DISCOVERED

#else
#include <libcups3/cups/cups.h>

#define KCUPS_PRINTER_COLOR CUPS_PTYPE_COLOR
#define KCUPS_PRINTER_SCANNER CUPS_PTYPE_SCANNER
#define KCUPS_PRINTER_REMOTE CUPS_PTYPE_REMOTE
#define KCUPS_PRINTER_LOCAL CUPS_PTYPE_LOCAL
#define KCUPS_PRINTER_CLASS CUPS_PTYPE_CLASS
#define KCUPS_PRINTER_DEFAULT CUPS_PTYPE_DEFAULT
#define KCUPS_PRINTER_DISCOVERED CUPS_PTYPE_DISCOVERED

#endif

class KCUPS_EXPORT KCupsCompat
{
public:
    explicit KCupsCompat() { };

    static const char *kcupsUser();
    static ipp_status_t kcupsError();
    static const char *kcupsErrorString();
    static int kcupsHttpReconnect(http_t *http, int msec, int *cancel);
    static ipp_attribute_t *kcupsIppFirstAttribute(ipp_t *response);
    static ipp_attribute_t *kcupsIppNextAttribute(ipp_t *response);
    static int kcupsIppPort();
    static http_t *
    kcupsHttpConnect(const char *host, int port, http_addrlist_t *addrlist, int family, http_encryption_t encryption, int blocking, int msec, int *cancel);
};
