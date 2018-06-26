
include_directories (
    ../library 
    ../widgets
    ${kmid_BINARY_DIR}/library
)

configure_file(config.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

###############
# main target #
###############

set(kmid_SRCS
   channels.cpp
   instrumentset.cpp
   kmid2.cpp
   main.cpp
   pianola.cpp
   playlistdialog.cpp
   ../widgets/keylabel.cpp
   ../widgets/pianokey.cpp
   ../widgets/pianokeybd.cpp
   ../widgets/pianoscene.cpp
   ../widgets/rhythmview.cpp
   ../widgets/timelabel.cpp
   ../widgets/vumeter.cpp
)

ki18n_wrap_ui( kmid_SRCS prefs_midi.ui prefs_lyrics.ui )
qt5_add_resources( kmid_SRCS ../widgets/pianokeybd.qrc )
# kf5_add_app_icon( kmid_SRCS "../icons/hi*-app-kmid.png" )
qt5_add_dbus_adaptor(kmid_SRCS org.kde.KMid.xml kmid2.h KMid2)

add_executable( kmid ${kmid_SRCS} )
target_link_libraries( kmid 
    KF5::KDELibs4Support
    KF5::WidgetsAddons
    Qt5::Svg
    kmidbackend
)

install( TARGETS kmid ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES kmid.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kmidui.rc DESTINATION ${DATA_INSTALL_DIR}/kmid )
install( FILES org.kde.KMid.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

#########
# kpart #
#########

set(kmid_part_SRCS
   kmid_partview.cpp
   kmid_part.cpp
)

qt5_add_dbus_adaptor(kmid_part_SRCS org.kde.KMidPart.xml kmid_part.h KMidPart)
add_library(kmid_part ${kmid_part_SRCS})
target_link_libraries( kmid_part
    KF5::KDELibs4Support
    KF5::MediaPlayer
    kmidbackend
)

install( TARGETS kmid_part DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kmid_part.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kmid_part.rc DESTINATION ${DATA_INSTALL_DIR}/kmid_part )
install( FILES org.kde.KMidPart.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
