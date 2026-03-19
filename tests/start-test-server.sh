#!/bin/sh

TESTDIR=/mock

exec ippserver -vvv -C $PWD$TESTDIR -r _print "$@"
