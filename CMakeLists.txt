cmake_minimum_required(VERSION 3.1)

# KDE Application Version, managed by release script
set (KDE_APPLICATIONS_VERSION_MAJOR "19")
set (KDE_APPLICATIONS_VERSION_MINOR "07")
set (KDE_APPLICATIONS_VERSION_MICRO "70")
set (KDE_APPLICATIONS_VERSION "${KDE_APPLICATIONS_VERSION_MAJOR}.${KDE_APPLICATIONS_VERSION_MINOR}.${KDE_APPLICATIONS_VERSION_MICRO}")

project(print-manager VERSION ${KDE_APPLICATIONS_VERSION})

set(QT_MIN_VERSION "5.9.0")
set(KF5_MIN_VERSION "5.42.0")



################# set KDE specific information #################

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMPackageConfigHelpers)
include(ECMOptionalAddSubdirectory)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Network
    Widgets
    Qml
    Quick
)

find_package(CUPS "1.5" REQUIRED)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED Config ConfigWidgets CoreAddons DBusAddons IconThemes I18n KCMUtils KIO
             Notifications Plasma WidgetsAddons WindowSystem)

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050900
    -DQT_USE_FAST_OPERATOR_PLUS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_CAST_FROM_ASCII
)

remove_definitions(-DQT_NO_KEYWORDS)

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)


include_directories(${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libkcups
    ${CUPS_INCLUDE_DIR}
)

add_definitions(-DTRANSLATION_DOMAIN="print-manager")

add_subdirectory(libkcups)
add_subdirectory(configure-printer)
add_subdirectory(add-printer)
add_subdirectory(printer-manager-kcm)
add_subdirectory(printqueue)
add_subdirectory(print-manager-kded)
add_subdirectory(plasmoid)
add_subdirectory(declarative-plugins)
