set(libqcups_SRCS
    cupsActions.cpp
    QCups.cpp
    ModifyPrinter.cpp
    ConfigureDialog.cpp
    PrinterPage.cpp
    PrinterOptions.cpp
    PrinterBehavior.cpp
    PPDModel.cpp
    SelectMakeModel.cpp
    CupsPasswordDialog.cpp
)

kde4_add_ui_files(libqcups_SRCS
    ModifyPrinter.ui
    PrinterOptions.ui
    PrinterBehavior.ui
    SelectMakeModel.ui
)

kde4_add_library(qcupslib SHARED ${libqcups_SRCS})
target_link_libraries(qcupslib
    ${KDE4_KIO_LIBS}
    ${CUPS_LIB}
)
# ${KDE4_KDEUI_LIBS}
install(TARGETS qcupslib ${INSTALL_TARGETS_DEFAULT_ARGS})
