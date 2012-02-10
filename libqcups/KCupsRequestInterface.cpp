#include "KCupsRequestInterface.h"

#include <KLocale>

KCupsRequestInterface::KCupsRequestInterface()
{
    connect(this, SIGNAL(finished()), &m_loop, SLOT(quit()));
}

void KCupsRequestInterface::invokeMethod(const char *method,
                                    const QVariant &arg1,
                                    const QVariant &arg2,
                                    const QVariant &arg3,
                                    const QVariant &arg4,
                                    const QVariant &arg5,
                                    const QVariant &arg6,
                                    const QVariant &arg7,
                                    const QVariant &arg8)
{
    m_error = false;
    m_errorMsg.clear();

    // If this fails we get into a infinite loop
    // Do not use global()->thread() which point
    // to the KCupsConnection parent thread
    moveToThread(KCupsConnection::global());

    m_finished = !QMetaObject::invokeMethod(this,
                                            method,
                                            Qt::QueuedConnection,
                                            QGenericArgument(arg1.typeName(), arg1.data()),
                                            QGenericArgument(arg2.typeName(), arg2.data()),
                                            QGenericArgument(arg3.typeName(), arg3.data()),
                                            QGenericArgument(arg4.typeName(), arg4.data()),
                                            QGenericArgument(arg5.typeName(), arg5.data()),
                                            QGenericArgument(arg6.typeName(), arg6.data()),
                                            QGenericArgument(arg7.typeName(), arg7.data()),
                                            QGenericArgument(arg8.typeName(), arg8.data()));
    if (m_finished) {
        setError(1, i18n("Failed to invoke method: %1").arg(method));
        setFinished();
    }
}

void KCupsRequestInterface::waitTillFinished()
{
    if (m_finished) {
        return;
    }

    m_loop.exec();
}

bool KCupsRequestInterface::hasError() const
{
    return m_error;
}

int KCupsRequestInterface::error() const
{
    return m_error;
}

QString KCupsRequestInterface::errorMsg() const
{
    return m_errorMsg;
}

void KCupsRequestInterface::setError(int error, const QString &errorMsg)
{
    m_error = error;
    m_errorMsg = errorMsg;
}

void KCupsRequestInterface::setFinished()
{
    m_finished = true;
    emit finished();
}
