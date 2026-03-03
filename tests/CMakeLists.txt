# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

ecm_add_test(modeltests.cpp
    TEST_NAME modeltests
    LINK_LIBRARIES kcups Qt::Test
    NAME_PREFIX "pm-"
)
