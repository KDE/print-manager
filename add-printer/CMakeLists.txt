find_package(KDE4Internal REQUIRED)

set(addprinter_SRCS
    main.cpp
    AddPrinter.cpp
    DevicesModel.cpp
    AddPrinterInterface.cpp
    PageIntro.cpp
    PageDestinations.cpp
)

kde4_add_ui_files(addprinter_SRCS
    PageIntro.ui
    PageDestinations.ui
)

qt4_add_dbus_adaptor(addprinter_dbus_SRCS
    org.kde.AddPrinter.xml
    AddPrinterInterface.h
    AddPrinterInterface
)

kde4_add_executable(add-printer
    ${addprinter_SRCS}
    ${addprinter_dbus_SRCS}
)

target_link_libraries(add-printer
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${CUPS_LIB}
    qcupslib
)

install(TARGETS add-printer DESTINATION ${LIBEXEC_INSTALL_DIR})

include(PkgConfigGetVar)
dbus_add_activation_service(org.kde.AddPrinter.service.in)
