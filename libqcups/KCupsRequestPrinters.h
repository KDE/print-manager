#ifndef KCUPSREQUESTPRINTERS_H
#define KCUPSREQUESTPRINTERS_H

#include "KCupsRequestInterface.h"
#include "KCupsConnection.h"

class KDE_EXPORT KCupsRequestPrinters : public KCupsRequestInterface
{
    Q_OBJECT
public:
    explicit KCupsRequestPrinters();

    void setAttributes(const QString &destName, bool isClass, const Arguments &values, const char *filename = NULL);
    void setShared(const QString &destName, bool isClass, bool shared);
    void getAttributes(const QString &destName, bool isClass, const QStringList &requestedAttr);
    void printTestPage(const QString &destName, bool isClass);
    void printCommand(const QString &destName, const QString &command, const QString &title);
    KIcon icon(const QString &destName, int printerType);

signals:

public slots:

};

#endif // KCUPSREQUESTPRINTERS_H
