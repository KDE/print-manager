set(printmanagerkded_SRCS
    Debug.cpp
    NewPrinterNotification.cpp
    PrintManagerKded.cpp
    Debug.h
    NewPrinterNotification.h
    PrintManagerKded.h
)

qt_add_dbus_adaptor(printmanagerkded_SRCS
    com.redhat.NewPrinterNotification.xml
    NewPrinterNotification.h
    NewPrinterNotification
)

add_library(kded_printmanager MODULE ${printmanagerkded_SRCS})
set_target_properties(kded_printmanager PROPERTIES OUTPUT_NAME printmanager)

target_link_libraries(kded_printmanager
    Qt::Core
    KF5::DBusAddons
    KF5::Notifications
    kcupslib
)

install(TARGETS kded_printmanager DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/kded)
install(FILES printmanager.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
