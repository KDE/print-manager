set(addprinter_SRCS
    main.cpp
    AddPrinter.cpp
    DevicesModel.cpp
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

    AddPrinter.h
    DevicesModel.h
    GenericPage.h
    PageDestinations.h
    ChooseLpd.h
    ChooseSamba.h
    ChooseSerial.h
    ChooseSocket.h
    ChooseUri.h
    PageChoosePrinters.h
    PageChoosePPD.h
    PageAddPrinter.h
    AddPrinterAssistant.h
)

ki18n_wrap_ui(addprinter_SRCS
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

add_executable(kde-add-printer
    ${addprinter_SRCS}
)

target_link_libraries(kde-add-printer
    Qt::Core
    Qt::Widgets
    Qt::Network
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::ConfigCore
    kcupslib
)
if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(kde-add-printer Qt6::Core5Compat)
endif()

install(TARGETS kde-add-printer DESTINATION ${KDE_INSTALL_BINDIR})
install(PROGRAMS org.kde.kde-add-printer.desktop DESTINATION ${KDE_INSTALL_APPDIR})
