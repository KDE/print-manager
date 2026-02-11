include(GenerateExportHeader)

set(libkcups_SRCS
    KIppRequest.cpp
    KCupsPasswordDialog.cpp
    KCupsConnection.cpp
    KCupsJob.cpp
    KCupsPrinter.cpp
    KCupsRequest.cpp
    PrinterModel.cpp
    JobModel.cpp
    JobSortFilterModel.cpp
    ProcessRunner.cpp

    KIppRequest.h
    KCupsPasswordDialog.h
    KCupsConnection.h
    KCupsJob.h
    KCupsPrinter.h
    KCupsServer.h
    KCupsRequest.h
    PrinterModel.h
    JobModel.h
    JobSortFilterModel.h
    ProcessRunner.h
)

if (NOT BUILD_WITH_CUPS3)
    set(libkcups_SRCS
        ${libkcups_SRCS}
        DevicesModel.cpp
        PPDModel.cpp
        KCupsServer.cpp

        DevicesModel.h
        PPDModel.h
        KCupsServer.h
    )
endif()

add_library(kcups SHARED ${libkcups_SRCS})

generate_export_header(kcups)

target_link_libraries(kcups PUBLIC
    Qt::Core
    Qt::DBus
    Qt::Qml
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::WidgetsAddons
    KF6::WindowSystem
    Cups::Cups
)

ecm_add_qml_module(kcups URI "org.kde.plasma.printmanager" GENERATE_PLUGIN_SOURCE)
ecm_finalize_qml_module(kcups)

ecm_qt_declare_logging_category(kcups
    HEADER kcupslib_log.h
    IDENTIFIER LIBKCUPS
    CATEGORY_NAME org.kde.plasma.printmanager.libkcups
    DESCRIPTION "Print Manager (libkcups)"
    EXPORT PMLOGS
)

install(TARGETS kcups ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

