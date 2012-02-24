include(PkgConfigGetVar)

set(configureprinter_SRCS
    ConfigurePrinterInterface.cpp
    ModifyPrinter.cpp
    PrinterPage.cpp
    PrinterOptions.cpp
    PrinterBehavior.cpp
    ConfigureDialog.cpp
    ConfigurePrinter.cpp
    main.cpp
)

kde4_add_ui_files(configureprinter_SRCS
    ModifyPrinter.ui
    PrinterOptions.ui
    PrinterBehavior.ui
)

qt4_add_dbus_adaptor(configureprinter_SRCS
    org.kde.ConfigurePrinter.xml
    ConfigurePrinterInterface.h
    ConfigurePrinterInterface
)

dbus_add_activation_service(org.kde.ConfigurePrinter.service.in)

kde4_add_executable(configure-printer ${configureprinter_SRCS})
target_link_libraries(configure-printer
    ${KDE4_KIO_LIBS}
    ${CUPS_LIB}
    qcupslib
)

install(TARGETS configure-printer DESTINATION ${LIBEXEC_INSTALL_DIR})
