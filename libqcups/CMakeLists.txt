set(libqcups_SRCS
    KCupsPasswordDialog.cpp
    KCupsConnection.cpp
    KCupsJob.cpp
    KCupsPrinter.cpp
    KCupsServer.cpp
    KCupsRequest.cpp
    ClassListWidget.cpp
    PPDModel.cpp
    SelectMakeModel.cpp
)

kde4_add_ui_files(libqcups_SRCS
    SelectMakeModel.ui
)

kde4_add_library(qcupslib SHARED ${libqcups_SRCS})
target_link_libraries(qcupslib
    ${KDE4_KIO_LIBS}
    ${CUPS_LIB}
)

install(TARGETS qcupslib ${INSTALL_TARGETS_DEFAULT_ARGS})
