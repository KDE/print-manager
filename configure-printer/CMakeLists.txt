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

configure_file(org.kde.ConfigurePrinter.service.in org.kde.ConfigurePrinter.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ConfigurePrinter.service DESTINATION ${DBUS_SERVICES_INSTALL_DIR})

kde4_add_executable(configure-printer ${configureprinter_SRCS})
target_link_libraries(configure-printer
    ${KDE4_KIO_LIBS}
    ${CUPS_LIB}
    kcupslib
)

install(TARGETS configure-printer DESTINATION ${LIBEXEC_INSTALL_DIR})
