/*
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KCupsCompat.h"

const char *KCupsCompat::kcupsUser()
{
#if CUPS_VERSION_MAJOR >= 3
    return cupsGetUser();
#else
    return cupsUser();
#endif
}

ipp_status_t KCupsCompat::kcupsError()
{
#if CUPS_VERSION_MAJOR >= 3
    return cupsGetError();
#else
    return cupsLastError();
#endif
}

const char *KCupsCompat::kcupsErrorString()
{
#if CUPS_VERSION_MAJOR >= 3
    return cupsGetErrorString();
#else
    return cupsLastErrorString();
#endif
}

http_t *KCupsCompat::kcupsHttpConnect(const char *host,
                                      int port,
                                      http_addrlist_t *addrlist,
                                      int family,
                                      http_encryption_t encryption,
                                      int blocking,
                                      int msec,
                                      int *cancel)
{
#if CUPS_VERSION_MAJOR >= 3
    return httpConnectEncrypt(host, port, encryption);
#else
    return httpConnect2(host, port, addrlist, family, encryption, blocking, msec, cancel);
#endif
}

int KCupsCompat::kcupsHttpReconnect(http_t *http, int msec, int *cancel)
{
#if CUPS_VERSION_MAJOR >= 3
    return httpConnectAgain(http, msec, cancel);
#else
    return httpReconnect2(http, msec, cancel);
#endif
}

ipp_attribute_t *KCupsCompat::kcupsIppFirstAttribute(ipp_t *response)
{
#if CUPS_VERSION_MAJOR >= 3
    return ippGetFirstAttribute(response);
#else
    return ippFirstAttribute(response);
#endif
}

ipp_attribute_t *KCupsCompat::kcupsIppNextAttribute(ipp_t *response)
{
#if CUPS_VERSION_MAJOR >= 3
    return ippGetNextAttribute(response);
#else
    return ippNextAttribute(response);
#endif
}

int KCupsCompat::kcupsIppPort()
{
#if CUPS_VERSION_MAJOR >= 3
    return ippGetPort();
#else
    return ippPort();
#endif
}
