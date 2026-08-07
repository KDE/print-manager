/*
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <kcups_export.h>

#ifdef LIBCUPS_VERSION_2
#include <cups/cups.h>

#define KCUPS_PRINTER_COLOR CUPS_PRINTER_COLOR
#define KCUPS_PRINTER_SCANNER CUPS_PRINTER_SCANNER
#define KCUPS_PRINTER_REMOTE CUPS_PRINTER_REMOTE
#define KCUPS_PRINTER_CLASS CUPS_PRINTER_CLASS
#define KCUPS_PRINTER_DEFAULT CUPS_PRINTER_DEFAULT
#else
#include <libcups3/cups/cups.h>

#define KCUPS_PRINTER_COLOR CUPS_PTYPE_COLOR
#define KCUPS_PRINTER_SCANNER CUPS_PTYPE_SCANNER
#define KCUPS_PRINTER_REMOTE CUPS_PTYPE_REMOTE
#define KCUPS_PRINTER_CLASS CUPS_PTYPE_CLASS
#define KCUPS_PRINTER_DEFAULT CUPS_PTYPE_DEFAULT
#endif

class KCUPS_EXPORT KCupsCompat
{
public:
    static const char *kcupsUser()
    {
#if CUPS_VERSION_MAJOR >= 3
        return cupsGetUser();
#else
        return cupsUser();
#endif
    }

    static void kcupsSetPasswordCB(cups_password_cb2_t cb, void *user_data)
    {
#if CUPS_VERSION_MAJOR >= 3
        cupsSetPasswordCB(cb, user_data);
#else
        cupsSetPasswordCB2(cb, user_data);
#endif
    }

    static ipp_status_t kcupsError()
    {
#if CUPS_VERSION_MAJOR >= 3
        return cupsGetError();
#else
        return cupsLastError();
#endif
    }

    static const char *kcupsErrorString()
    {
#if CUPS_VERSION_MAJOR >= 3
        return cupsGetErrorString();
#else
        return cupsLastErrorString();
#endif
    }

    static int kcupsHttpReconnect(http_t *http, int msec, int *cancel)
    {
#if CUPS_VERSION_MAJOR >= 3
        return httpConnectAgain(http, msec, cancel);
#else
        return httpReconnect2(http, msec, cancel);
#endif
    }

private:
    KCupsCompat();
};
