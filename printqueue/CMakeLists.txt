set(queuemanager_SRCS
    main.cpp
    PrintQueue.cpp
    PrintQueueUi.cpp
)

ki18n_wrap_ui(queuemanager_SRCS
    PrintQueueUi.ui
)

add_executable(kde-print-queue
    ${queuemanager_SRCS}
)

target_link_libraries(kde-print-queue
    Qt5::Core
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WidgetsAddons
    KF5::DBusAddons
    KF5::ConfigCore
    ${CUPS_LIB}
    kcupslib
)

install(TARGETS kde-print-queue DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.PrintQueue.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
