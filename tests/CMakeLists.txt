# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>

ecm_add_test(modeltests.cpp
    TEST_NAME modeltests
    LINK_LIBRARIES kcupslib Qt::Test
    NAME_PREFIX "pm-"
)

if (LOCAL_ONLY_TESTS)
    ecm_add_test(devicemodeltests.cpp
        TEST_NAME devicemodeltests
        LINK_LIBRARIES kcupslib Qt::Test
        NAME_PREFIX "pm-"
    )
endif()
