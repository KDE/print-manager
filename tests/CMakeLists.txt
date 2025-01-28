# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>

ecm_add_test(servertest.cpp
    TEST_NAME cupsservertest
    LINK_LIBRARIES kcupslib Qt::Test
    NAME_PREFIX "pm-"
)
ecm_add_test(callkcmtest.cpp
    TEST_NAME callkcmtest
    LINK_LIBRARIES kcupslib Qt::Test
    NAME_PREFIX "pm-"
)
ecm_add_test(kdedtest.cpp
    TEST_NAME kdedtest
    LINK_LIBRARIES kcupslib Qt::Test
    NAME_PREFIX "pm-"
)
ecm_add_test(modeltests.cpp
    TEST_NAME modeltests
    LINK_LIBRARIES kcupslib Qt::Test
    NAME_PREFIX "pm-"
)

