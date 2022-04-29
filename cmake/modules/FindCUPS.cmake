# SPDX-FileCopyrightText: 2010-2012 Daniel Nicolett <dantti12@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

#[=======================================================================[.rst:
FindCUPS
--------

Finds the CUPS library.

# Imported Targets
# ^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``CUPS::CUPS``
  The CUPS library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``CUPS_FOUND``
  True if the system has the CUPS library.
``CUPS_VERSION``
  The version of the CUPS library which was found.
``CUPS_INCLUDE_DIRS``
  Include directories needed to use CUPS.
``CUPS_LIBRARIES``
  Libraries needed to link to CUPS.
``CUPS_DEFINITIONS``
  Compiler switches needed to use CUPS.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``CUPS_INCLUDE_DIR``
  The directory containing ``cups/cups.h``.
``CUPS_LIBRARY``
  The path to the CUPS library.

#]=======================================================================]

find_package(PkgConfig)
pkg_check_modules(PC_CUPS QUIET cups)

find_path(CUPS_INCLUDE_DIR
  NAMES cups.h
  PATHS ${PC_CUPS_INCLUDE_DIRS}
  PATH_SUFFIXES cups
)

find_library(CUPS_LIBRARY
  NAMES cups
  PATHS ${PC_CUPS_LIBRARY_DIRS}
)

set(CUPS_VERSION ${PC_CUPS_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CUPS
  FOUND_VAR CUPS_FOUND
  REQUIRED_VARS
    CUPS_LIBRARY
    CUPS_INCLUDE_DIR
  VERSION_VAR CUPS_VERSION
)

if(CUPS_FOUND)
  set(CUPS_LIBRARIES ${CUPS_LIBRARY})
  set(CUPS_INCLUDE_DIRS ${CUPS_INCLUDE_DIR})
  set(CUPS_DEFINITIONS ${PC_CUPS_CFLAGS_OTHER})
endif()

if (CUPS_INCLUDE_DIR AND CUPS_LIB)
    set(CUPS_FIND_QUIETLY TRUE)
endif()

if(CUPS_FOUND AND NOT TARGET CUPS::CUPS)
  add_library(CUPS::CUPS UNKNOWN IMPORTED)
  set_target_properties(CUPS::CUPS PROPERTIES
    IMPORTED_LOCATION "${CUPS_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${CUPS_DEFINITIONS}"
    INTERFACE_INCLUDE_DIRECTORIES "${CUPS_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(
  CUPS_INCLUDE_DIR
  CUPS_LIBRARY
)
