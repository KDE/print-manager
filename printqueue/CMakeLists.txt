find_package(KDE4Internal REQUIRED)
include(PkgConfigGetVar)

set(queuemanager_SRCS
    main.cpp
    PrintQueue.cpp
    PrintQueueModel.cpp
    PrintQueueUi.cpp
    PrintQueueInterface.cpp
    PrintQueueSortFilterProxyModel.cpp
)

kde4_add_ui_files(queuemanager_SRCS
    PrintQueueUi.ui
)

qt4_add_dbus_adaptor(queuemanager_SRCS
    org.kde.PrintQueue.xml
    PrintQueueInterface.h
    PrintQueueInterface
)

dbus_add_activation_service(org.kde.PrintQueue.service.in)

kde4_add_executable(print-queue
    ${queuemanager_SRCS}
)

target_link_libraries(print-queue
    ${KDE4_KDEUI_LIBS}
    qcupslib
)

install(TARGETS print-queue DESTINATION ${LIBEXEC_INSTALL_DIR})
