message (STATUS "building a local copy of drumstick")

include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# drumstick-file library

SET(drumstick-file_QTOBJ_SRCS
    include/qsmf.h
    include/qwrk.h
)

SET(drumstick-file_HEADERS
    include/qsmf.h
    include/qwrk.h
)

SET(drumstick-file_SRCS
    src/qsmf.cpp
    src/qwrk.cpp
)

QT4_WRAP_CPP(drumstick-file_MOC_SRCS ${drumstick-file_QTOBJ_SRCS})

ADD_LIBRARY(drumstick-file
    ${drumstick-file_MOC_SRCS}
    ${drumstick-file_SRCS} 
    ${drumstick-file_HEADERS} 
)

IF(NOT WIN32)
    SET_TARGET_PROPERTIES(drumstick-file PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF(NOT WIN32)

TARGET_LINK_LIBRARIES(drumstick-file
    ${QT_LIBRARIES}
)

# drumstick-alsa library
if (ALSA_FOUND)

    include_directories (
        ${ALSA_INCLUDEDIR}
    )
    
    SET(drumstick-alsa_QTOBJ_SRCS
        include/alsaclient.h
        include/alsaevent.h
        include/alsaport.h
        include/alsaqueue.h
        include/alsatimer.h
        include/playthread.h
    )
    
    SET(drumstick-alsa_HEADERS
        include/alsaclient.h
        include/alsaevent.h
        include/alsaport.h
        include/alsaqueue.h
        include/alsatimer.h
        include/drumstickcommon.h
        include/playthread.h
        include/subscription.h
    )
    
    SET(drumstick-alsa_SRCS
        src/alsaclient.cpp
        src/alsaevent.cpp
        src/alsaport.cpp
        src/alsaqueue.cpp
        src/alsatimer.cpp
        src/playthread.cpp
        src/subscription.cpp
    )
    
    QT4_WRAP_CPP(drumstick-alsa_MOC_SRCS ${drumstick-alsa_QTOBJ_SRCS})
       
    ADD_LIBRARY(drumstick-alsa 
        ${drumstick-alsa_MOC_SRCS}
        ${drumstick-alsa_SRCS} 
        ${drumstick-alsa_HEADERS} 
    )
    
    SET_TARGET_PROPERTIES(drumstick-alsa PROPERTIES COMPILE_FLAGS -fPIC)
    
    TARGET_LINK_LIBRARIES(drumstick-alsa
        ${QT_LIBRARIES}
        ${ALSA_LIBS}
    )

endif (ALSA_FOUND)
