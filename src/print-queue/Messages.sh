/*
 *   SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#!/usr/bin/env bash

$XGETTEXT `find . -name '*.cpp' -o -name '*.qml'` -o $podir/org.kde.plasma.printqueue.pot
