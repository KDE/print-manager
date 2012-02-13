#include "KCupsPrinter.h"

KCupsPrinter::KCupsPrinter() :
    m_isClass(false)
{
}

KCupsPrinter::KCupsPrinter(const QString &printer, bool isClass) :
    m_printer(printer),
    m_isClass(isClass)
{
}

KCupsPrinter::KCupsPrinter(const Arguments &arguments) :
    m_arguments(arguments)
{
    m_printer = arguments["printer-name"].toString();
    m_isClass = arguments["printer-type"].toInt() & CUPS_PRINTER_CLASS;
}

QString KCupsPrinter::name() const
{
    return m_printer;
}

bool KCupsPrinter::isClass() const
{
    return m_isClass;
}

bool KCupsPrinter::isDefault() const
{
    return m_arguments["printer-type"].toUInt() & CUPS_PRINTER_DEFAULT;
}

bool KCupsPrinter::isShared() const
{
    return m_arguments["printer-is-shared"].toBool();
}

cups_ptype_e KCupsPrinter::type() const
{
    return static_cast<cups_ptype_e>(m_arguments["printer-type"].toUInt());
}

QString KCupsPrinter::location() const
{
    return m_arguments["printer-location"].toString();
}

QString KCupsPrinter::description() const
{
    return m_arguments["printer-info"].toString();
}

QString KCupsPrinter::makeAndModel() const
{
    return m_arguments["printer-make-and-model"].toString();
}

QStringList KCupsPrinter::commands() const
{
    return m_arguments["printer-commands"].toStringList();
}

int KCupsPrinter::state() const
{
    return m_arguments["printer-state"].toInt();
}

QString KCupsPrinter::stateMsg() const
{
    return m_arguments["printer-state-message"].toString();
}

int KCupsPrinter::markerChangeTime() const
{
    return m_arguments["marker-change-time"].toInt();
}

QVariant KCupsPrinter::argument(const QString &name) const
{
    return m_arguments.value(name);
}
