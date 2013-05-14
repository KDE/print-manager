set(printmanager_qml_plugin_SRCS
   qmlplugins.cpp
)

kde4_add_library(printmanager SHARED ${printmanager_qml_plugin_SRCS})
target_link_libraries(printmanager
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_PLASMA_LIBS}
    kcupslib
)

install(TARGETS printmanager DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/printmanager)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/printmanager)
