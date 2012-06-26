# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
 
# We add our source code here
set(printers_engine_SRCS
    PrintersServiceJob.cpp
    PrintersService.cpp
    PrintersEngine.cpp
)
 
# Now make sure all files get to the right place
kde4_add_plugin(plasma_engine_printers ${printers_engine_SRCS})
target_link_libraries(plasma_engine_printers
    ${KDE4_KDECORE_LIBS}
    ${KDE4_PLASMA_LIBS}
    kcupslib
)
 
install(TARGETS plasma_engine_printers
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-engine-printers.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES org.kde.printers.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
