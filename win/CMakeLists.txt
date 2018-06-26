message (STATUS "building a Windows backend for KMid")

include_directories(
    ../library
    ${kmid_BINARY_DIR}/library
    ${DRUMSTICK_INCLUDEDIR}
)

set( plugin_SRCS
    winbackend.cpp
    winmidiobject.cpp
    winmidioutput.cpp
    song.cpp
)

set ( WINDOWS_LIBRARIES
    winmm
)

kde4_add_plugin( kmid_win ${plugin_SRCS} )

target_link_libraries( kmid_win
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${WINDOWS_LIBRARIES}
    ${DRUMSTICK_LIBRARIES}
    kmidbackend
)

install( TARGETS kmid_win DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kmid_win.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
