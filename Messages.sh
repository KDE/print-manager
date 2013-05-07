#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui"` >> rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/print-manager.pot
rm -f rc.cpp

$XGETTEXT `find plasmoid -name \*.cpp` -o $podir/plasma_applet_printmanager.pot
$XGETTEXT `find plasmoid -name \*.qml` -j -L Java -o $podir/plasma_applet_printmanager.pot
