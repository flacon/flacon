if (NOT CMAKE_BUILD_TYPE)
    set ( CMAKE_BUILD_TYPE Release )
endif (NOT CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE MATCHES [Dd]ebug)
    add_definitions("-g")
endif()

set(STATUS_MESSAGES "")


macro(status_message message)
  set(STATUS_MESSAGES ${STATUS_MESSAGES} ${message})
endmacro()


macro(show_status)
    message(STATUS "*****************************************************")

    foreach(msg ${STATUS_MESSAGES})
        message(STATUS "* ${msg}")
        message(STATUS "*")
    endforeach()

    message(STATUS "*****************************************************")
endmacro()


macro(setByDefault VAR_NAME VAR_VALUE)
  if (NOT DEFINED ${VAR_NAME})
    set (${VAR_NAME} ${VAR_VALUE})
  endif()
  add_definitions(-D${VAR_NAME}=\"${VAR_VALUE}\")
endmacro()
