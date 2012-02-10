#ifndef KCUPSREQUESTSERVER_H
#define KCUPSREQUESTSERVER_H

#include "KCupsRequestInterface.h"

class KCupsRequestServer : public KCupsRequestInterface
{
    Q_OBJECT
public:
    explicit KCupsRequestServer(QObject *parent = 0);

    void adminSetServerSettings(const HashStrStr &userValues);
    void getPPDS(const QString &make = QString());

    void getDevices();
    // THIS function can get the default server dest through the
    // "printer-is-default" attribute BUT it does not get user
    // defined default printer, see cupsGetDefault() on www.cups.org for details
    void getDests(int mask, const QStringList &requestedAttr = QStringList());
    void getJobs(const QString &destName, bool myJobs, int whichJobs, const QStringList &requestedAttr = QStringList());

    void addClass(const QHash<QString, QVariant> &values);

    /*
     The result will be in hashStrStr()
    */
    void adminGetServerSettings();
};

#endif // KCUPSREQUESTSERVER_H
