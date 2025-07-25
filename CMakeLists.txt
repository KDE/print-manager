cmake_minimum_required(VERSION 3.16)

project(print-manager)
set(PROJECT_VERSION "6.4.80")

set(PROJECT_DEP_VERSION "6.3.90")
set(QT_MIN_VERSION "6.8.0")
set(KF6_MIN_VERSION "6.14.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEGitCommitHooks)
include(KDEClangFormat)

include(ECMDeprecationSettings)
include(ECMQmlModule)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Network
    Widgets
    Qml
    Quick
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED 
    Config 
    ConfigWidgets 
    CoreAddons 
    DBusAddons 
    I18n 
    KCMUtils 
    Kirigami
    KIO
    Notifications 
    WidgetsAddons 
    WindowSystem 
)

find_package(Plasma ${PROJECT_DEP_VERSION} REQUIRED)

ecm_set_disabled_deprecation_versions(
    QT 6.8
    KF 6.12.0
)

set_package_properties(KF6Kirigami PROPERTIES
    TYPE REQUIRED
    PURPOSE "Kirigami application UI framework"
)

find_package(PackageKitQt6)
set_package_properties(PackageKitQt6
    PROPERTIES DESCRIPTION "Software Manager integration"
    TYPE OPTIONAL
    PURPOSE "Needed to automatically install the system-config-printer package."
)

find_package(Cups "2.4" REQUIRED)
message(STATUS "Detected CUPS version: ${CUPS_VERSION_STRING}")
#add declaration for libcups version
if(CUPS_VERSION_STRING VERSION_GREATER_EQUAL "3.0")
    message(STATUS "Building with libcups3")
    add_definitions(-DLIBCUPS_VERSION_3)
else()
    message(STATUS "Building with libcups2")
    add_definitions(-DLIBCUPS_VERSION_2)
endif()

find_package(CupsSmb)
set_package_properties(CupsSmb PROPERTIES
    TYPE RUNTIME
    DESCRIPTION "SMB backend for CUPS (aka smbspool)"
    PURPOSE "Required for CUPS to be able to print to printers shared over SMB"
    URL "https://samba.org"
)

option(SCP_INSTALL "Offer to install System-config-printer with PackageKit if it is not already installed, use -DSCP_INSTALL=off to disable, use -DSCP_PACKAGE_NAME= to set package name." ON)
set(SCP_PACKAGE_NAME \"system-config-printer\" CACHE STRING "Single package or comma-separated list of packages needed for a functional System-config-printer on this distribution.")

if(SCP_INSTALL AND PackageKitQt6_FOUND)
    add_definitions(-DSCP_INSTALL)
    add_definitions(-DSCP_PACKAGE_NAME=${SCP_PACKAGE_NAME})
else()
    set(SCP_INSTALL false)
endif()
add_feature_info("System-config-printer Installation" SCP_INSTALL "Automatic installation of '${SCP_PACKAGE_NAME}' using PackageKit.")

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)


include_directories(${CMAKE_BINARY_DIR}) # config.h

install(FILES org.kde.print-manager.metainfo.xml DESTINATION "${KDE_INSTALL_METAINFODIR}")

add_definitions(-DTRANSLATION_DOMAIN="print-manager")

add_subdirectory(src)

ecm_qt_install_logging_categories(
    EXPORT PMLOGS
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

ki18n_install(po)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
