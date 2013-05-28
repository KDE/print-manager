find_package(KDE4Internal REQUIRED)

set(queuemanager_SRCS
    main.cpp
    PrintQueue.cpp
    PrintQueueUi.cpp
)

kde4_add_ui_files(queuemanager_SRCS
    PrintQueueUi.ui
)

kde4_add_executable(kde-print-queue
    ${queuemanager_SRCS}
)

target_link_libraries(kde-print-queue
    ${KDE4_KDEUI_LIBS}
    kcupslib
)

install(TARGETS kde-print-queue DESTINATION ${BIN_INSTALL_DIR})
