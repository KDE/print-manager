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
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::ConfigCore
    ${CUPS_LIB}
    kcupslib
)

install(TARGETS kde-add-printer DESTINATION ${BIN_INSTALL_DIR})
install(PROGRAMS org.kde.AddPrinter.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
