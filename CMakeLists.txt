cmake_minimum_required(VERSION 3.16)

project(print-manager)
set(PROJECT_VERSION "5.92.90")

set(PROJECT_DEP_VERSION "6.0.80")
set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "5.240.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################# set KDE specific information #################

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEGitCommitHooks)
include(KDEClangFormat)

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
find_package(Qt6Core5Compat)

find_package(Cups "1.5" REQUIRED)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED 
    Config 
    ConfigWidgets 
    CoreAddons 
    DBusAddons 
    IconThemes 
    I18n 
    KCMUtils 
    Kirigami
    KIO
    Notifications 
    WidgetsAddons 
    WindowSystem 
)

find_package(Plasma ${PROJECT_DEP_VERSION} REQUIRED)

set_package_properties(KF6Kirigami PROPERTIES
    TYPE REQUIRED
    PURPOSE "Kirigami application UI framework"
)

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

add_subdirectory(src)

ecm_qt_install_logging_categories(
    EXPORT PMLOGS
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

ki18n_install(po)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
