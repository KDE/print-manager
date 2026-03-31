# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

ecm_add_test(
    subscriptiontest.cpp
    TEST_NAME subscriptiontests
    LINK_LIBRARIES kcups Qt::Test
    NAME_PREFIX "pm-"
)

ecm_add_test(
    modeltest.cpp
    TEST_NAME modeltests
    LINK_LIBRARIES kcups Qt::Test
    NAME_PREFIX "pm-"
)

ecm_add_test(
    commandtest.cpp
    TestHelpers.h
    TEST_NAME commandtests
    LINK_LIBRARIES kcups Qt::Test
    NAME_PREFIX "pm-"
)

add_custom_command(TARGET modeltests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts
            $<TARGET_FILE_DIR:modeltests>/scripts
)
