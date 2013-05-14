set(printmanager_applet_SRCS
    PlasmoidConfig.cpp
    printmanager.cpp
)

kde4_add_ui_files(printmanager_applet_SRCS
    PlasmoidConfig.ui
)

kde4_add_plugin(plasma_applet_printmanager ${printmanager_applet_SRCS})

target_link_libraries(plasma_applet_printmanager
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    ${QT_QTDECLARATIVE_LIBRARY}
    kcupslib
)

#install(TARGETS plasma_applet_printmanager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES package/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-printmanager.desktop)
install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.printmanager)
