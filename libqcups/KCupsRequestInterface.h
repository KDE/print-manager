/***************************************************************************
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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
    void setError(int error, const QString &errorMsg);
    void setFinished();

    QEventLoop m_loop;
    bool m_finished;
    int m_error;
    QString m_errorMsg;
    ReturnArguments m_retArguments;
};

#endif // KCUPSREQUESTINTERFACE_H
