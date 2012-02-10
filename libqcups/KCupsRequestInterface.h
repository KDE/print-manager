#ifndef KCUPSREQUESTINTERFACE_H
#define KCUPSREQUESTINTERFACE_H

#include <QObject>
#include <QEventLoop>

#include "KCupsConnection.h"

class KDE_EXPORT KCupsRequestInterface : public QObject
{
    Q_OBJECT
public:
    void waitTillFinished();

    bool hasError() const;
    int error() const;
    QString errorMsg() const;
    ReturnArguments result() const;
    HashStrStr hashStrStr() const;

signals:
    void finished();

protected:
    KCupsRequestInterface();
    void invokeMethod(const char *method,
                      const QVariant &arg1 = QVariant(),
                      const QVariant &arg2 = QVariant(),
                      const QVariant &arg3 = QVariant(),
                      const QVariant &arg4 = QVariant(),
                      const QVariant &arg5 = QVariant(),
                      const QVariant &arg6 = QVariant(),
                      const QVariant &arg7 = QVariant(),
                      const QVariant &arg8 = QVariant());
    void setHashStrStr(const HashStrStr &hash);
    void setError(int error, const QString &errorMsg);
    void setFinished();

    QEventLoop m_loop;
    bool m_finished;
    int m_error;
    QString m_errorMsg;
    HashStrStr m_hash;
    ReturnArguments m_retArguments;
};

#endif // KCUPSREQUESTINTERFACE_H
