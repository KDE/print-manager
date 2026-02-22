#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>
# SPDX-License-Identifier: LGPL-2.0-or-later

$XGETTEXT `find . -name '*.cpp' -o -name '*.qml'` -o $podir/org.kde.plasma.printqueue.pot
