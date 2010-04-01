set(printd_SRCS
    printd.cpp
    PrintQueueTray.cpp)

kde4_add_plugin(kded_printd ${printd_SRCS})

target_link_libraries(kded_printd
    ${KDE4_KIO_LIBS}
    ${CUPS_LIB}
    ${KDE4_SOLID_LIBS}
)

install(TARGETS kded_printd DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES printd.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)