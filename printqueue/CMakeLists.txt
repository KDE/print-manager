find_package(KDE4Internal REQUIRED)

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

qt4_add_dbus_adaptor(queuemanager_dbus_SRCS
    org.kde.PrintQueue.xml
    PrintQueueInterface.h
    PrintQueueInterface
)

kde4_add_executable(print-queue
    ${queuemanager_SRCS}
    ${queuemanager_dbus_SRCS}
)

target_link_libraries(print-queue
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${CUPS_LIB}
    qcupslib
)

install(TARGETS print-queue DESTINATION ${LIBEXEC_INSTALL_DIR})

install(FILES PrintQueue.notifyrc DESTINATION ${DATA_INSTALL_DIR}/PrintQueue)

# include(PkgConfigGetVar)
# dbus_add_activation_service(org.kde.PrintQueue.service.in)
