#ifndef KCUPSPRINTER_H
#define KCUPSPRINTER_H

#include <QString>
#include <KCupsConnection.h>

class KDE_EXPORT KCupsPrinter
{
public:
    KCupsPrinter();
    KCupsPrinter(const QString &printer, bool isClass);

    QString name() const;
    bool isClass() const;
    bool isDefault() const;
    bool isShared() const;
    cups_ptype_e type() const;
    QString location() const;
    QString description() const;
    QString makeAndModel() const;
    QStringList commands() const;

    int state() const;
    QString stateMsg() const;
    int markerChangeTime() const;
    QVariant argument(const QString &name) const;

protected:
    KCupsPrinter(const Arguments &arguments);

private:
    friend class KCupsRequestServer;

    QString m_printer;
    bool    m_isClass;
    Arguments m_arguments;
};

#endif // KCUPSPRINTER_H
