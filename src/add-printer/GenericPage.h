/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GENERIC_PAGE_H
#define GENERIC_PAGE_H

#include <QWidget>
#include <QHash>
#include <QVariant>

#define ADDING_PRINTER         QLatin1String("add-new-printer")
#define PPD_NAME               QLatin1String("ppd-name")
#define FILENAME               QLatin1String("filename")

class GenericPage : public QWidget
{
    Q_OBJECT
public:
    explicit GenericPage(QWidget *parent = nullptr);
    virtual bool canProceed() const { return true; }
    virtual bool isValid() const { return true; }
    virtual bool isWorking() const { return m_working; }
    virtual void setValues(const QVariantHash &args);
    virtual QVariantHash values() const;

    virtual bool finishClicked() { return false; }

signals:
    void allowProceed(bool allow);
    void proceed();
    void startWorking();
    void stopWorking();

protected slots:
    void working();
    void notWorking();

protected:
    QVariantHash m_args;
    int m_working;
};

#endif
