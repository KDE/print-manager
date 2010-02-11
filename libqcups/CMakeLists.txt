set(libqcups_SRCS
    QCups.cpp
    cupsActions.cpp
)

kde4_add_library(qcupslib SHARED ${libqcups_SRCS})
target_link_libraries(qcupslib
    ${KDE4_KIO_LIBS}
    ${CUPS_LIB}
)

install(TARGETS qcupslib ${INSTALL_TARGETS_DEFAULT_ARGS})
