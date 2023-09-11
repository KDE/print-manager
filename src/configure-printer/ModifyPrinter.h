/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MODIFY_PRINTER_H
#define MODIFY_PRINTER_H

#include "PrinterPage.h"

#include <QWidget>

namespace Ui {
    class ModifyPrinter;
}

class ModifyPrinter : public PrinterPage
{
    Q_OBJECT
public:
    enum Role{
        PPDDefault,
        PPDCustom,
        PPDFile,
        PPDList,
        PPDName = Qt::UserRole + 1
    };
    Q_ENUM(Role)

    explicit ModifyPrinter(const QString &destName, bool isClass, QWidget *parent = nullptr);
    ~ModifyPrinter() override;

    bool hasChanges() override;
    QVariantMap modifiedValues() const override;
    QStringList neededValues() const override;
    void setRemote(bool remote) override;

    void setValues(const KCupsPrinter &printer);
    void setCurrentMake(const QString &make);
    void setCurrentMakeAndModel(const QString &makeAndModel);

    void save() override;

signals:
    void ppdChanged();

private slots:
    void textChanged(const QString &text);
    void makeActivated(int index);
    void ppdSelectionAccepted();
    void ppdSelectionRejected();
    void modelChanged();

private:
    Ui::ModifyPrinter *ui;
    QString m_destName, m_make, m_makeAndModel;
    bool m_isClass;
    QVariantMap m_changedValues;
    int m_changes = 0;
};

#endif
