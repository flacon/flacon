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

  ${CMAKE_CURRENT_LIST_DIR}/movie.h
  ${CMAKE_CURRENT_LIST_DIR}/movie.cpp

  ${CMAKE_CURRENT_LIST_DIR}/appimage.qrc
  ${CMAKE_CURRENT_LIST_DIR}/appimage.css
)

include(${CMAKE_CURRENT_LIST_DIR}/aboutdialog/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/coverdialog/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/logview/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/messagebox/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/preferences/module.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/tageditor/module.cmake)
