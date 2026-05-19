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

# kcmshell6 < 6.27.0 doesn't know the --smoke-test param, so skip this test for
# now.
# TODO: Drop this when KF6_MIN_VERSION >= 6.27.0
if(KF6KCMUtils_VERSION VERSION_GREATER_EQUAL "6.27.0")
    add_test(NAME kcm_printer_manager_smoketest COMMAND kcmshell6 --smoke-test kcm_printer_manager)
    set_tests_properties(kcm_printer_manager_smoketest PROPERTIES
        ENVIRONMENT_MODIFICATION QT_PLUGIN_PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin
    )
endif()

add_test(NAME print-queue_smoketest COMMAND plasma-print-queue --smoke-test)
set_tests_properties(print-queue_smoketest PROPERTIES
    ENVIRONMENT_MODIFICATION QT_PLUGIN_PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin
)

add_custom_command(TARGET modeltests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts
            $<TARGET_FILE_DIR:modeltests>/scripts
)
