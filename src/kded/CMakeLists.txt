set(printmanagerkded_SRCS
    NewPrinterNotification.cpp
    PrintManagerKded.cpp
    NewPrinterNotification.h
    PrintManagerKded.h
    MarkerLevelChecker.cpp
    MarkerLevelChecker.h
)

qt_add_dbus_adaptor(printmanagerkded_SRCS
    com.redhat.NewPrinterNotification.xml
    NewPrinterNotification.h
    NewPrinterNotification
)

kcoreaddons_add_plugin(kded_printmanager INSTALL_NAMESPACE "kf6/kded" SOURCES ${printmanagerkded_SRCS})
set_target_properties(kded_printmanager PROPERTIES OUTPUT_NAME printmanager)

target_link_libraries(kded_printmanager PRIVATE
    Qt6::Core
    KF6::DBusAddons
    KF6::Notifications
    kcups
)

ecm_qt_declare_logging_category(kded_printmanager
    HEADER pmkded_log.h
    IDENTIFIER PMKDED
    CATEGORY_NAME org.kde.plasma.printmanager.kded
    DESCRIPTION "Print Manager (kded)"
    EXPORT PMLOGS
)

install(FILES printmanager.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
