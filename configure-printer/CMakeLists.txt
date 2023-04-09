set(configureprinter_SRCS
    ConfigurePrinterInterface.cpp
    Debug.cpp
    ModifyPrinter.cpp
    PrinterPage.cpp
    PrinterOptions.cpp
    PrinterBehavior.cpp
    ConfigureDialog.cpp
    ConfigurePrinter.cpp
    SelectMakeModelDialog.cpp
    main.cpp
)

ki18n_wrap_ui(configureprinter_SRCS
    ModifyPrinter.ui
    PrinterOptions.ui
    PrinterBehavior.ui
)

qt_add_dbus_adaptor(configureprinter_SRCS
    org.kde.ConfigurePrinter.xml
    ConfigurePrinterInterface.h
    ConfigurePrinterInterface
)

add_executable(configure-printer ${configureprinter_SRCS})
target_link_libraries(configure-printer
    Qt::Core
    Qt::Widgets
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::IconThemes
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::KIOWidgets
    KF${QT_MAJOR_VERSION}::WidgetsAddons
    KF${QT_MAJOR_VERSION}::DBusAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    Cups::Cups
    kcupslib
)

if(${QT_MAJOR_VERSION} EQUAL "6")
    target_link_libraries(configure-printer
        Qt6::Core5Compat
    )
endif()

install(TARGETS configure-printer DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.ConfigurePrinter.desktop DESTINATION ${KDE_INSTALL_APPDIR})
