# Comment
list(APPEND SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/profilespage.h
  ${CMAKE_CURRENT_LIST_DIR}/profilespage.cpp
  ${CMAKE_CURRENT_LIST_DIR}/profilespage.ui


  ${CMAKE_CURRENT_LIST_DIR}/profiletabwidget.h
  ${CMAKE_CURRENT_LIST_DIR}/profiletabwidget.cpp
  ${CMAKE_CURRENT_LIST_DIR}/profiletabwidget.ui

  ${CMAKE_CURRENT_LIST_DIR}/addprofiledialog.h
  ${CMAKE_CURRENT_LIST_DIR}/addprofiledialog.cpp
  ${CMAKE_CURRENT_LIST_DIR}/addprofiledialog.ui

  ${CMAKE_CURRENT_LIST_DIR}/covergroupbox.h
  ${CMAKE_CURRENT_LIST_DIR}/covergroupbox.cpp
  ${CMAKE_CURRENT_LIST_DIR}/covergroupbox.ui

  ${CMAKE_CURRENT_LIST_DIR}/cuegroupbox.h
  ${CMAKE_CURRENT_LIST_DIR}/cuegroupbox.cpp
  ${CMAKE_CURRENT_LIST_DIR}/cuegroupbox.ui
)

include_directories(${CMAKE_CURRENT_LIST_DIR})
