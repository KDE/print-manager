include(ECMQmlModule)
include(ECMGenerateQmlTypes)

ecm_add_qml_module(printmanager URI "org.kde.plasma.printmanager")

target_sources(printmanager PRIVATE
   qmlplugins.cpp
)

target_link_libraries(printmanager
    Qt6::Qml
    Qt6::Quick
    kcupslib
)

ecm_finalize_qml_module(printmanager DESTINATION ${KDE_INSTALL_QMLDIR})
