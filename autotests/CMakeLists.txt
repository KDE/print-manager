# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>

ecm_add_test(modeltests.cpp
    TEST_NAME modeltests
    LINK_LIBRARIES kcupslib Qt::Test
    NAME_PREFIX "pm-"
)
