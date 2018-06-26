message ( STATUS "building the Mac OSX backend for KMid" )

include_directories (
    ../library
    ${kmid_BINARY_DIR}/library
)

set ( plugin_SRCS
    macosxbackend.cpp
    macmidiobject.cpp
    macmidioutput.cpp
    dlssoftsynth.cpp
)

set ( DARWIN_LIBRARIES 
    "-framework CoreAudio"
    "-framework CoreMIDI"
    "-framework CoreFoundation"
    "-framework AudioToolbox"
    "-framework AudioUnit"
)

kde4_add_ui_files( plugin_SRCS prefs_synth.ui )

kde4_add_plugin ( kmid_mac ${plugin_SRCS} )

target_link_libraries ( kmid_mac
    ${KDE4_KDEUI_LIBS} 
    ${KDE4_KIO_LIBS} 
    ${DARWIN_LIBRARIES}
    kmidbackend
)

install ( TARGETS kmid_mac DESTINATION ${PLUGIN_INSTALL_DIR} )
install ( FILES kmid_mac.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
