project(print-manager)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/)

find_package(KDE4 REQUIRED)
find_package(CUPS "1.4" REQUIRED)

include(FindPkgConfig)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/libqcups
)

add_subdirectory(libqcups)
add_subdirectory(add-printer)
add_subdirectory(printer-manager-kcm)
add_subdirectory(printqueue)
add_subdirectory(printd)
add_subdirectory(printmanager-dataengine)

install(DIRECTORY plasmoid-package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/printmanager)

install(FILES plasmoid-package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-printmanager.desktop)
