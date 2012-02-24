set(newprinternotification_SRCS
    NewPrinterNotification.cpp
)

qt4_add_dbus_adaptor(newprinternotification_SRCS
    com.redhat.NewPrinterNotification.xml
    NewPrinterNotification.h
    NewPrinterNotification
)

kde4_add_plugin(kded_newprinternotification ${newprinternotification_SRCS})

target_link_libraries(kded_newprinternotification
    ${KDE4_KDEUI_LIBS}
    qcupslib
)

install(TARGETS kded_newprinternotification DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES newprinternotification.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
install(FILES NewPrinterNotification.notifyrc DESTINATION ${DATA_INSTALL_DIR}/NewPrinterNotification)
