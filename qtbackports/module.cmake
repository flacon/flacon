list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/movetotrash.h
    ${CMAKE_CURRENT_LIST_DIR}/movetotrash_unix.cpp

)

if (APPLE)
    list(APPEND SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/movetotrash_mac.mm
    )
endif()
