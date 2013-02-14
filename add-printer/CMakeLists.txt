find_package(KDE4Internal REQUIRED)

set(addprinter_SRCS
    main.cpp
    AddPrinter.cpp
    DevicesModel.cpp
    AddPrinterInterface.cpp
    GenericPage.cpp
    PageDestinations.cpp
    ChooseLpd.cpp
    ChooseSamba.cpp
    ChooseSerial.cpp
    ChooseSocket.cpp
    ChooseUri.cpp
    PageChoosePrinters.cpp
    PageChoosePPD.cpp
    PageAddPrinter.cpp
    AddPrinterAssistant.cpp
)

kde4_add_ui_files(addprinter_SRCS
    PageDestinations.ui
    ChooseLpd.ui
    ChooseSamba.ui
    ChooseSerial.ui
    ChooseSocket.ui
    ChooseUri.ui
    PageChoosePrinters.ui
    PageChoosePPD.ui
    PageAddPrinter.ui
)

qt4_add_dbus_adaptor(addprinter_SRCS
    org.kde.AddPrinter.xml
    AddPrinterInterface.h
    AddPrinterInterface
)

kde4_add_executable(add-printer
    ${addprinter_SRCS}
)

target_link_libraries(add-printer
    ${KDE4_KDEUI_LIBS}
    ${QT_QTNETWORK_LIBRARY}
    ${KDE4_KIO_LIBS}
    kcupslib
)

install(TARGETS add-printer DESTINATION ${LIBEXEC_INSTALL_DIR})

configure_file(org.kde.AddPrinter.service.in org.kde.AddPrinter.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.AddPrinter.service DESTINATION ${DBUS_SERVICES_INSTALL_DIR})
