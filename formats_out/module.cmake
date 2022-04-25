list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/outformat.h
    ${CMAKE_CURRENT_LIST_DIR}/outformat.cpp

    ${CMAKE_CURRENT_LIST_DIR}/encoderconfigpage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encoderconfigpage.h

    ${CMAKE_CURRENT_LIST_DIR}/metadata.h
    ${CMAKE_CURRENT_LIST_DIR}/metadata.cpp
)

include(${CMAKE_CURRENT_LIST_DIR}/aac/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/alac/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/flac/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/mp3/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/ogg/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/opus/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/wav/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/wv/module.cmake)
