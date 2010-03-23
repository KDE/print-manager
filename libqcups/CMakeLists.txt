set(libqcups_SRCS
    QCups.cpp
    cupsActions.cpp
    ModifyPrinter.cpp
    ConfigureDialog.cpp
    PrinterPage.cpp
    PrinterBehavior.cpp
)

kde4_add_ui_files(libqcups_SRCS
    ModifyPrinter.ui
    PrinterBehavior.ui
)

kde4_add_library(qcupslib SHARED ${libqcups_SRCS})
target_link_libraries(qcupslib
    ${KDE4_KIO_LIBS}
    ${CUPS_LIB}
)
# ${KDE4_KDEUI_LIBS}
install(TARGETS qcupslib ${INSTALL_TARGETS_DEFAULT_ARGS})
