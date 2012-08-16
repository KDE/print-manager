set(libkcups_SRCS
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
)

kde4_add_ui_files(libkcups_SRCS
    SelectMakeModel.ui
)

kde4_add_library(kcupslib SHARED ${libkcups_SRCS})
target_link_libraries(kcupslib
    ${KDE4_KIO_LIBS}
    ${CUPS_LIB}
)

install(TARGETS kcupslib ${INSTALL_TARGETS_DEFAULT_ARGS})
