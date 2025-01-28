# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>

add_executable(callkcmtests callkcmtests.cpp)
target_link_libraries(callkcmtests
    kcups
)

add_executable(cupsservertests cupsservertests.cpp)
target_link_libraries(cupsservertests
    kcups
)

add_executable(kdedtests kdedtests.cpp)
target_link_libraries(kdedtests
    kcups
)

add_executable(modeltests modeltests.cpp)
target_link_libraries(modeltests
    kcups
)
