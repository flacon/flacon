list(APPEND SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/controls.h
  ${CMAKE_CURRENT_LIST_DIR}/controls.cpp

  ${CMAKE_CURRENT_LIST_DIR}/icon.h
  ${CMAKE_CURRENT_LIST_DIR}/icon.cpp

  ${CMAKE_CURRENT_LIST_DIR}/mainwindow.h
  ${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp
  ${CMAKE_CURRENT_LIST_DIR}/mainwindow.ui

  ${CMAKE_CURRENT_LIST_DIR}/trackview.h
  ${CMAKE_CURRENT_LIST_DIR}/trackview.cpp

  ${CMAKE_CURRENT_LIST_DIR}/trackviewdelegate.h
  ${CMAKE_CURRENT_LIST_DIR}/trackviewdelegate.cpp

  ${CMAKE_CURRENT_LIST_DIR}/trackviewmodel.cpp
  ${CMAKE_CURRENT_LIST_DIR}/trackviewmodel.h
)

include(${CMAKE_CURRENT_LIST_DIR}/preferences/module.cmake)


