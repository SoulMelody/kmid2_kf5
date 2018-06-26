project (kmid)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(VERSION 2.4.0)

# Building standalone ?
if (NOT KDE4_FOUND)
  set(BUILDING_STANDALONE true)
  #find_package (KDE4 REQUIRED)
  #include (KDE4Defaults)
endif (NOT KDE4_FOUND)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

find_package(Qt5 REQUIRED COMPONENTS Widgets DBus Core Gui Svg)
find_package(KF5 REQUIRED COMPONENTS CoreAddons Solid Config I18n Plasma KDE4Support MediaPlayer WidgetsAddons)

include_directories ( ${KF5_INCLUDES} ${QT_INCLUDES} )

# Linux ?
if ( CMAKE_SYSTEM MATCHES "Linux")
  find_package (PkgConfig)
  if (PKG_CONFIG_FOUND)
    # Check for ALSA 1.0
    pkg_check_modules (ALSA alsa>=1.0)
    if (ALSA_FOUND)
      set ( WITH_ALSA 1 )
      set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )
      # Check for DRUMSTICK
      pkg_check_modules(DRUMSTICK-FILE drumstick-file>=0.4)
      pkg_check_modules(DRUMSTICK-ALSA drumstick-alsa>=0.4)
      if (DRUMSTICK-ALSA_FOUND AND DRUMSTICK-FILE_FOUND)
        # build the local copy
        #add_definitions (-DDRUMSTICK_STATIC)
        set (DRUMSTICK_LIBRARIES drumstick-file drumstick-alsa ${ALSA_LIBRARIES})
        set (DRUMSTICK_INCLUDEDIR "/usr/include/drumstick")
        #add_subdirectory (drumstick)
      endif (DRUMSTICK-ALSA_FOUND AND DRUMSTICK-FILE_FOUND)
    endif (ALSA_FOUND)
  endif (PKG_CONFIG_FOUND)
endif ( CMAKE_SYSTEM MATCHES "Linux")

# Mac OSX ?
if ( CMAKE_SYSTEM MATCHES "Darwin")
  include(CheckIncludeFile)
  check_include_file(CoreMIDI/MIDIServices.h WITH_COREMIDI)
  check_include_file(CoreAudio/AudioHardware.h WITH_COREAUDIO)
endif ( CMAKE_SYSTEM MATCHES "Darwin")

# Windows ?
if ( CMAKE_SYSTEM MATCHES "Windows" )
  include(CheckIncludeFiles)
  check_include_files ( windows.h WINDOWS_FOUND )
  if (WINDOWS_FOUND)
    check_include_files( "windows.h;mmsystem.h" WITH_WINMM )
  endif (WINDOWS_FOUND)
  # PkgConfig in windows, TODO: better check
  find_package (PkgConfig)
  if (PKG_CONFIG_FOUND)
    # Check for drumstick-file
    pkg_check_modules(DRUMSTICK drumstick-file>=0.4)
  endif (PKG_CONFIG_FOUND)
  if (NOT DRUMSTICK_FOUND)
    add_definitions (-DDRUMSTICK_STATIC)
    set (DRUMSTICK_LIBRARIES drumstick-file)
    set (DRUMSTICK_INCLUDEDIR "${CMAKE_CURRENT_SOURCE_DIR}/drumstick/include")
    add_subdirectory (drumstick)
  endif (NOT DRUMSTICK_FOUND)
endif ( CMAKE_SYSTEM MATCHES "Windows" )

# common backend classes
add_subdirectory ( library )

# Main program
add_subdirectory ( src )

# Platform native backends
if (ALSA_FOUND)
  add_subdirectory (alsa)
endif (ALSA_FOUND)

if (WITH_COREAUDIO AND WITH_COREMIDI)
  add_subdirectory ( mac )
endif (WITH_COREAUDIO AND WITH_COREMIDI)

if ( WINDOWS_FOUND AND WITH_WINMM )
  add_subdirectory ( win )
endif ( WINDOWS_FOUND AND WITH_WINMM )

# Test backend
# add_subdirectory ( dummy )

add_subdirectory ( icons )
add_subdirectory ( maps )
add_subdirectory ( examples )
add_subdirectory(po)
# add_subdirectory(doc)
