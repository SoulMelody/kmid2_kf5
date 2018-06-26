message (STATUS "building the ALSA backend for KMid")

include_directories( 
    ../library
    ${ALSA_INCLUDEDIR}
    ${DRUMSTICK_INCLUDEDIR}
    ${kmid_BINARY_DIR}/library
)

set( plugin_SRCS
    alsabackend.cpp
    alsamidiobject.cpp
    alsamidioutput.cpp
    externalsoftsynth.cpp
    song.cpp
    player.cpp
)

ki18n_wrap_ui( plugin_SRCS prefs_progs.ui )

add_library( kmid_alsa ${plugin_SRCS} ) 

target_link_libraries( kmid_alsa 
    KF5::KDELibs4Support
    ${DRUMSTICK_LIBRARIES}
    kmidbackend
)

install( TARGETS kmid_alsa DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES kmid_alsa.desktop DESTINATION ${SERVICES_INSTALL_DIR})
