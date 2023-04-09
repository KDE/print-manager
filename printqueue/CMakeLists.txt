set(queuemanager_SRCS
    main.cpp
    PrintQueue.cpp
    PrintQueueUi.cpp
    PrintQueue.h
    PrintQueueUi.h
)

ki18n_wrap_ui(queuemanager_SRCS
    PrintQueueUi.ui
)

add_executable(kde-print-queue
    ${queuemanager_SRCS}
)

target_link_libraries(kde-print-queue
    Qt::Core
    Qt::Widgets
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::KIOWidgets
    KF${QT_MAJOR_VERSION}::WidgetsAddons
    KF${QT_MAJOR_VERSION}::DBusAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    kcupslib
)

install(TARGETS kde-print-queue DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.PrintQueue.desktop DESTINATION ${KDE_INSTALL_APPDIR})
