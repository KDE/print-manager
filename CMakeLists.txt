cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "90")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(print-manager VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.2")
set(KF_MIN_VERSION "5.98.0")


################# set KDE specific information #################

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMOptionalAddSubdirectory)
include(FeatureSummary)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Network
    Widgets
    Qml
    Quick
)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat)
endif()

find_package(Cups "1.5" REQUIRED)

find_package(KF5 ${KF_MIN_VERSION} REQUIRED Config ConfigWidgets CoreAddons DBusAddons IconThemes I18n KCMUtils KIO
             Notifications Plasma WidgetsAddons WindowSystem)

find_package(CupsSmb)
set_package_properties(CupsSmb PROPERTIES
    TYPE RUNTIME
    DESCRIPTION "SMB backend for CUPS (aka smbspool)"
    PURPOSE "Required for CUPS to be able to print to printers shared over SMB"
    URL "https://samba.org"
)

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


include_directories(${CMAKE_BINARY_DIR}) # config.h

install(FILES org.kde.print-manager.metainfo.xml DESTINATION "${KDE_INSTALL_METAINFODIR}")

add_definitions(-DTRANSLATION_DOMAIN="print-manager")

add_subdirectory(libkcups)
add_subdirectory(configure-printer)
add_subdirectory(add-printer)
add_subdirectory(printer-manager-kcm)
add_subdirectory(printqueue)
add_subdirectory(print-manager-kded)
add_subdirectory(plasmoid)
add_subdirectory(declarative-plugins)

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
