set(kcm_print_SRCS
    PrinterModel.cpp
    PrinterDelegate.cpp
    PrinterDescription.cpp
    PrintKCM.cpp
    SystemPreferences.cpp
)

kde4_add_ui_files(kcm_print_SRCS
    PrintKCM.ui
    PrinterDescription.ui
    SystemPreferences.ui
    ServerError.ui
)

kde4_add_plugin(kcm_printer_manager ${kcm_print_SRCS})

target_link_libraries(kcm_printer_manager
    ${KDE4_KDEUI_LIBS}
    qcupslib
)

install(TARGETS kcm_printer_manager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_printer_manager.desktop DESTINATION ${SERVICES_INSTALL_DIR})
