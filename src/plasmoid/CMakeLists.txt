# SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>
# SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

plasma_add_applet(org.kde.plasma.printmanager
    QML_SOURCES
        main.qml
        PrinterDelegate.qml
        FullRepresentation.qml
    RESOURCES main.xml
    GENERATE_APPLET_CLASS
)
