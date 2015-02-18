#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui" | grep -v "plasmoid"` >> rc.cpp
$XGETTEXT `find . -name "*.cpp" | grep -v "plasmoid"` -o $podir/print-manager.pot
rm -f rc.cpp

$XGETTEXT `find plasmoid -name "*.qml"` -L Java -o $podir/plasma_applet_org.kde.plasma.printmanager.pot
