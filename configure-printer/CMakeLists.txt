include_directories(${CMAKE_SOURCE_DIR}/libkcups)

set(configureprinter_SRCS
    ConfigurePrinterInterface.cpp
#     ModifyPrinter.cpp
#     PrinterPage.cpp
#     PrinterOptions.cpp
#     PrinterBehavior.cpp
#     ConfigureDialog.cpp
#     ConfigurePrinter.cpp
#     main.cpp
)

ki18n_wrap_ui(configureprinter_SRCS
    ModifyPrinter.ui
    PrinterOptions.ui
    PrinterBehavior.ui
)

qt5_add_dbus_adaptor(configureprinter_SRCS
    org.kde.ConfigurePrinter.xml
    ConfigurePrinterInterface.h
    ConfigurePrinterInterface
)

configure_file(org.kde.ConfigurePrinter.service.in org.kde.ConfigurePrinter.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ConfigurePrinter.service DESTINATION ${DBUS_SERVICES_INSTALL_DIR})

add_executable(configure-printer ${configureprinter_SRCS})
target_link_libraries(configure-printer
    Qt5::Core
    Qt5::Widgets
    KF5::Completion
    KF5::I18n
    KF5::IconThemes
    KF5::KDELibs4Support
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WidgetsAddons
    ${CUPS_LIB}
    kcupslib
)

install(TARGETS configure-printer DESTINATION ${LIBEXEC_INSTALL_DIR})
