set(cupsd_SRCS
    printd.cpp
    PrintQueueTray.cpp)

kde4_add_plugin(kded_cupsd ${cupsd_SRCS})

target_link_libraries(kded_cupsd
    ${KDE4_KIO_LIBS}
    ${CUPS_LIB}
    ${KDE4_SOLID_LIBS}
)

install(TARGETS kded_cupsd DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES printd.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
