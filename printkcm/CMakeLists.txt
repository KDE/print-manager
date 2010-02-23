set(kcm_print_SRCS
    PrintKCM.cpp
)

kde4_add_ui_files(kcm_print_SRCS
    PrintKCM.ui
)

kde4_add_plugin(kcm_print ${kcm_print_SRCS})

target_link_libraries(kcm_print
    ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_print DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES settings-hardware.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES printer-manager.desktop DESTINATION ${SERVICES_INSTALL_DIR})
