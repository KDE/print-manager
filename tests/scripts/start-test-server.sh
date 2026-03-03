#!/bin/sh

# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

TESTDIR=/mock

exec ippserver -vvv -C $PWD$TESTDIR -r _print "$@"
