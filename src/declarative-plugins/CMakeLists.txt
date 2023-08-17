set(printmanager_qml_plugin_SRCS
   qmlplugins.cpp
   qmlplugins.h
)

add_library(printmanager SHARED ${printmanager_qml_plugin_SRCS})
target_link_libraries(printmanager
    Qt::Qml
    Qt::Quick
    kcupslib
)

install(TARGETS printmanager DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/printmanager)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/printmanager)
