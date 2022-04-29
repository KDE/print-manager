set(libkcups_SRCS
    Debug.cpp
    KIppRequest.cpp
    KCupsPasswordDialog.cpp
    KCupsConnection.cpp
    KCupsJob.cpp
    KCupsPrinter.cpp
    KCupsServer.cpp
    KCupsRequest.cpp
    ClassListWidget.cpp
    PPDModel.cpp
    SelectMakeModel.cpp
    NoSelectionRectDelegate.cpp
    PrinterModel.cpp
    PrinterSortFilterModel.cpp
    JobModel.cpp
    JobSortFilterModel.cpp
    ProcessRunner.cpp
)

ki18n_wrap_ui(libkcups_SRCS
    SelectMakeModel.ui
)

add_library(kcupslib SHARED ${libkcups_SRCS})

target_link_libraries(kcupslib
    Qt::Core
    Qt::DBus
    Qt::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WidgetsAddons
    KF5::WindowSystem
    PkgConfig::CUPS
)

install(TARGETS kcupslib ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
