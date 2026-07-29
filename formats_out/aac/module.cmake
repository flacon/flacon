list(APPEND SOURCES

  ${CMAKE_CURRENT_LIST_DIR}/out_aac.h
  ${CMAKE_CURRENT_LIST_DIR}/out_aac.cpp

  ${CMAKE_CURRENT_LIST_DIR}/configpage_acc.h
  ${CMAKE_CURRENT_LIST_DIR}/configpage_acc.cpp
)

include(${CMAKE_CURRENT_LIST_DIR}/faac/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/fdkaac/module.cmake)


