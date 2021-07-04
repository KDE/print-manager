/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti
    dantti12@gmail.com

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ConfigurePrinter.h"

#include "ConfigurePrinterInterface.h"
#include "Debug.h"

#include <QTimer>

ConfigurePrinter::ConfigurePrinter(int & argc, char ** argv) :
    QApplication(argc, argv)
{
}

void ConfigurePrinter::configurePrinter(const QString& printer)
{
    m_cpInterface = new ConfigurePrinterInterface(this);
    connect(m_cpInterface, &ConfigurePrinterInterface::quit, this, &ConfigurePrinter::quit);

    if (!printer.isEmpty()) {
        m_cpInterface->ConfigurePrinter(printer);
    } else {
        // If DBus called the ui list won't be empty
        QTimer::singleShot(500, m_cpInterface, &ConfigurePrinterInterface::RemovePrinter);
    }
}

ConfigurePrinter::~ConfigurePrinter()
{
}

#include "moc_ConfigurePrinter.cpp"
