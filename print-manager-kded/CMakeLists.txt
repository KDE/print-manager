set(printmanagerkded_SRCS
    Debug.cpp
    NewPrinterNotification.cpp
    PrintManagerKded.cpp
)

qt5_add_dbus_adaptor(printmanagerkded_SRCS
    com.redhat.NewPrinterNotification.xml
    NewPrinterNotification.h
    NewPrinterNotification
)

add_library(kded_printmanager MODULE ${printmanagerkded_SRCS})

target_link_libraries(kded_printmanager
    Qt5::Core
    KF5::DBusAddons
    KF5::Notifications
    kcupslib
)

install(TARGETS kded_printmanager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES printmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
install(FILES printmanager.notifyrc DESTINATION ${DATA_INSTALL_DIR}/printmanager)
