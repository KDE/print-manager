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

qt4_add_dbus_adaptor(queuemanager_SRCS
    org.kde.PrintQueue.xml
    PrintQueueInterface.h
    PrintQueueInterface
)

configure_file(org.kde.PrintQueue.service.in org.kde.PrintQueue.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.PrintQueue.service DESTINATION ${DBUS_SERVICES_INSTALL_DIR})

kde4_add_executable(print-queue
    ${queuemanager_SRCS}
)

target_link_libraries(print-queue
    ${KDE4_KDEUI_LIBS}
    kcupslib
)

install(TARGETS print-queue DESTINATION ${LIBEXEC_INSTALL_DIR})
