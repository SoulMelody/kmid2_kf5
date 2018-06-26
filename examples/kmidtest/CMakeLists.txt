cmake_minimum_required(VERSION 2.6)

find_package (KDE4 REQUIRED)
include (KDE4Defaults)
include_directories ( ${KDE4_INCLUDES} ${QT_INCLUDES} )

set(kmidtest_SRCS kmidtest.cpp )
kde4_add_executable(kmidtest ${kmidtest_SRCS})
target_link_libraries(kmidtest 
    ${KDE4_KDEUI_LIBS} 
    ${KDE4_KPARTS_LIBS}
    kmediaplayer
)
install( TARGETS kmidtest ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES kmidtest.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kmidtestui.rc DESTINATION  ${DATA_INSTALL_DIR}/kmidtest )
