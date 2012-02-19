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
set(printmanager_engine_SRCS
    PrintManagerServiceJob.cpp
    PrintManagerService.cpp
    PrintManagerEngine.cpp
)
 
# Now make sure all files get to the right place
kde4_add_plugin(plasma_engine_printmanager ${printmanager_engine_SRCS})
target_link_libraries(plasma_engine_printmanager
    ${KDE4_KDECORE_LIBS}
    ${KDE4_PLASMA_LIBS}
    qcupslib
)
 
install(TARGETS plasma_engine_printmanager
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-engine-printmanager.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES printmanager.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
