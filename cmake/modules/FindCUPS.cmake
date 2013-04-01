#/***************************************************************************
# *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
# *   dantti12@gmail.com                                                    *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU General Public License for more details.                          *
# *                                                                         *
# *   You should have received a copy of the GNU General Public License     *
# *   along with this program; see the file COPYING. If not, write to       *
# *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
# *   Boston, MA 02110-1301, USA.                                           *
# ***************************************************************************/
# - Try to find CUPS
# Once done this will define
#
#  CUPS_FOUND - system has CUPS
#  CUPS_INCLUDE_DIR - the CUPS include directory
#  CUPS_LIB - Link these to use CUPS
#  CUPS_DEFINITIONS - Compiler switches required for using CUPS

# Copyright (c) 2010, Daniel Nicolett, <dantti12@gmail.com>
#
# Redistribution and use is allowed according to the terms of the GPLv2+ license.

IF (CUPS_INCLUDE_DIR AND CUPS_LIB)
    SET(CUPS_FIND_QUIETLY TRUE)
ENDIF (CUPS_INCLUDE_DIR AND CUPS_LIB)

FIND_PATH( CUPS_INCLUDE_DIR cups)

FIND_LIBRARY(CUPS_LIB NAMES cups)

IF (CUPS_INCLUDE_DIR AND CUPS_LIB)
   SET(CUPS_FOUND TRUE)
ELSE (CUPS_INCLUDE_DIR AND CUPS_LIB)
   SET(CUPS_FOUND FALSE)
ENDIF (CUPS_INCLUDE_DIR AND CUPS_LIB)

SET(CUPS_INCLUDE_DIR ${CUPS_INCLUDE_DIR})

IF (CUPS_FOUND)
  IF (NOT CUPS_FIND_QUIETLY)
    MESSAGE(STATUS "Found CUPS: ${CUPS_LIB}")
  ENDIF (NOT CUPS_FIND_QUIETLY)
ELSE (CUPS_FOUND)
  IF (CUPS_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could NOT find CUPS libraries")
  ENDIF (CUPS_FIND_REQUIRED)
ENDIF (CUPS_FOUND)

MARK_AS_ADVANCED(CUPS_INCLUDE_DIR CUPS_LIB)

