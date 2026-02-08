# SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

# Based on FindCups.cmake, distributed by https://cmake.org

# FindCups3
# --------
# 
# Finds the Common UNIX Printing System (CUPS):
# 
#   find_package(Cups3 [<version>] [...])
# 
# Imported Targets
# ^^^^^^^^^^^^^^^^
# 
# This module provides the following :ref:`Imported Targets`:
# 
# ``Cups::Cups``
# 
#   Target encapsulating the CUPS usage requirements, available only if CUPS is
#   found.
# 
# Result Variables
# ^^^^^^^^^^^^^^^^
# 
# This module defines the following variables:
# 
# ``Cups3_FOUND``
# 
#   Boolean indicating whether (the requested version of) CUPS was found.
# 
# ``Cups3_VERSION``
# 
#   The version of CUPS found.
# 
# ``CUPS_INCLUDE_DIRS``
#   Include directories needed for using CUPS.
# 
# Cache Variables
# ^^^^^^^^^^^^^^^
# 
# The following cache variables may also be set:
# 
# ``CUPS_INCLUDE_DIR``
#   The directory containing the CUPS headers.
# 
# ``CUPS_LIBRARIES``
#   Libraries needed to link against to use CUPS.
# 
# Examples
# ^^^^^^^^
# 
# Finding CUPS and linking it to a project target:
# 
# 
#   find_package(Cups3)
#   target_link_libraries(project_target PRIVATE Cups::Cups)

cmake_policy(PUSH)
cmake_policy(SET CMP0159 NEW) # file(STRINGS) with REGEX updates CMAKE_MATCH_<n>

find_path(CUPS_INCLUDE_DIR libcups3/cups/cups.h )

find_library(CUPS_LIBRARIES NAMES cups3 )

if (CUPS_INCLUDE_DIR AND EXISTS "${CUPS_INCLUDE_DIR}/libcups3/cups/cups.h")
    file(STRINGS "${CUPS_INCLUDE_DIR}/libcups3/cups/cups.h" cups_version_str
         REGEX "^#[\t ]*define[\t ]+CUPS_VERSION_(MAJOR|MINOR|PATCH)[\t ]+[0-9]+$")

    unset(Cups3_VERSION)
    foreach(VPART MAJOR MINOR PATCH)
        foreach(VLINE ${cups_version_str})
            if(VLINE MATCHES "^#[\t ]*define[\t ]+CUPS_VERSION_${VPART}[\t ]+([0-9]+)$")
                set(CUPS_VERSION_PART "${CMAKE_MATCH_1}")
                if(Cups3_VERSION)
                    string(APPEND Cups3_VERSION ".${CUPS_VERSION_PART}")
                else()
                    set(Cups3_VERSION "${CUPS_VERSION_PART}")
                endif()
            endif()
        endforeach()
    endforeach()
    set(CUPS_VERSION_STRING ${Cups3_VERSION})
endif ()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Cups3
                                  REQUIRED_VARS CUPS_LIBRARIES CUPS_INCLUDE_DIR
                                  VERSION_VAR Cups3_VERSION)

mark_as_advanced(CUPS_INCLUDE_DIR CUPS_LIBRARIES)

if (Cups3_FOUND)
    set(CUPS_INCLUDE_DIRS "${CUPS_INCLUDE_DIR}")
    if (NOT TARGET Cups::Cups)
        add_library(Cups::Cups INTERFACE IMPORTED)
        set_target_properties(Cups::Cups PROPERTIES
            INTERFACE_LINK_LIBRARIES      "${CUPS_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${CUPS_INCLUDE_DIR}")
    endif ()
endif ()

cmake_policy(POP)

