# - Try to find CUPS
# Once done this will define
#
#  CUPS_FOUND - system has CUPS
#  CUPS_INCLUDE_DIR - the CUPS include directory
#  CUPS_LIB - Link these to use CUPS
#  CUPS_DEFINITIONS - Compiler switches required for using CUPS

# SPDX-FileCopyrightText: 2010-2012 Daniel Nicolett <dantti12@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

if (CUPS_INCLUDE_DIR AND CUPS_LIB)
    set(CUPS_FIND_QUIETLY TRUE)
endif()

find_path(CUPS_INCLUDE_DIR cups)

find_library(CUPS_LIB NAMES cups)

if (CUPS_INCLUDE_DIR AND CUPS_LIB)
   set(CUPS_FOUND TRUE)
else()
   set(CUPS_FOUND FALSE)
endif()

set(CUPS_INCLUDE_DIR ${CUPS_INCLUDE_DIR})

if (CUPS_FOUND)
  if (NOT CUPS_FIND_QUIETLY)
    message(STATUS "Found CUPS: ${CUPS_LIB}")
  endif()
else()
  if (CUPS_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find CUPS libraries")
  endif()
endif()

mark_as_advanced(CUPS_INCLUDE_DIR CUPS_LIB)
