kcmutils_add_qml_kcm(kcm_printer_manager SOURCES 
    printermanager.cpp 
    printermanager.h
)

target_link_libraries(kcm_printer_manager PRIVATE
    Qt6::Qml
    Qt6::DBus
    KF6::KCMUtilsQuick
    KF6::I18n
    kcupslib
)
