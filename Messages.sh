#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui"` >> rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/print-manager.pot
rm -f rc.cpp

$XGETTEXT `find . -name \*.qml` -o $podir/plasma_applet_printmanager.pot
