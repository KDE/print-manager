include(GenerateExportHeader)

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

    Debug.h
    KIppRequest.h
    KCupsPasswordDialog.h
    KCupsConnection.h
    KCupsJob.h
    KCupsPrinter.h
    KCupsServer.h
    KCupsRequest.h
    ClassListWidget.h
    PPDModel.h
    SelectMakeModel.h
    NoSelectionRectDelegate.h
    PrinterModel.h
    PrinterSortFilterModel.h
    JobModel.h
    JobSortFilterModel.h
    ProcessRunner.h
)

ki18n_wrap_ui(libkcups_SRCS
    SelectMakeModel.ui
)

add_library(kcupslib SHARED ${libkcups_SRCS})

generate_export_header(kcupslib)

target_link_libraries(kcupslib
    Qt::Core
    Qt::DBus
    Qt::Widgets
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::WidgetsAddons
    KF6::WindowSystem
    Cups::Cups
)

install(TARGETS kcupslib ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
