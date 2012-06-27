set(printmanagerkded_SRCS
    NewPrinterNotification.cpp
    PrintManagerKded.cpp
)

qt4_add_dbus_adaptor(printmanagerkded_SRCS
    com.redhat.NewPrinterNotification.xml
    NewPrinterNotification.h
    NewPrinterNotification
)

kde4_add_plugin(kded_printmanager ${printmanagerkded_SRCS})

target_link_libraries(kded_printmanager
    ${KDE4_KDEUI_LIBS}
    kcupslib
)

install(TARGETS kded_printmanager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES printmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
install(FILES printmanager.notifyrc DESTINATION ${DATA_INSTALL_DIR}/printmanager)
