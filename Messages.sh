#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui" | grep -v "plasmoid"` >> rc.cpp
$XGETTEXT `find . -name "*.cpp" | grep -v "plasmoid"` -o $podir/print-manager.pot
rm -f rc.cpp

$EXTRACTRC `find plasmoid -name "*.rc" -o -name "*.ui"` >> rc.cpp
$XGETTEXT rc.cpp `find plasmoid -name "*.cpp"` -o $podir/plasma_applet_org.kde.printmanager.pot
$XGETTEXT `find plasmoid -name "*.qml"` -j -L Java -o $podir/plasma_applet_org.kde.printmanager.pot
