 # BEGIN_COMMON_COPYRIGHT_HEADER
 # (c)LGPL2+
 #
 # Flacon - audio File Encoder
 # https://github.com/flacon/flacon
 #
 # Copyright: 2012-2013
 #   Alexander Sokoloff <sokoloff.a@gmail.com>
 #
 # This library is free software; you can redistribute it and/or
 # modify it under the terms of the GNU Lesser General Public
 # License as published by the Free Software Foundation; either
 # version 2.1 of the License, or (at your option) any later version.

 # This library is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # Lesser General Public License for more details.

 # You should have received a copy of the GNU Lesser General Public
 # License along with this library; if not, write to the Free Software
 # Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 #
 # END_COMMON_COPYRIGHT_HEADER


project(flacon_test)
cmake_minimum_required( VERSION 3.16.0 )
cmake_policy(SET CMP0028 NEW)


set(TEST_SOURCES
    flacontest.h
    flacontest.cpp

    tools.h
    discspec.h
    tools.cpp
    inittestcase.cpp

    discspec.cpp
    convertertest.h
    convertertest.cpp
)

file(GLOB TEST_CPP_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp
)

list(APPEND TEST_SOURCES ${TEST_CPP_FILES})



foreach(FILE ${SOURCES} ${HEADERS})
    if(NOT FILE STREQUAL "main.cpp")
        if (IS_ABSOLUTE ${FILE})
            set(TEST_SOURCES ${TEST_SOURCES} "${FILE}")
        else()
           set(TEST_SOURCES ${TEST_SOURCES} "../${FILE}")
       endif()
    endif()
endforeach()

add_executable(${PROJECT_NAME} ${TEST_SOURCES}
    testspec.h testspec.cpp)

add_definitions(-DTEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\")

set(TEST_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/output/")
file(MAKE_DIRECTORY ${TEST_OUT_DIR})
add_definitions(-DTEST_OUT_DIR=\"${TEST_OUT_DIR}\")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)


add_definitions(-Wall)

find_package(PkgConfig REQUIRED)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED
    Core
    Test
    Widgets
    Network
)

pkg_search_module(YAML_CPP REQUIRED yaml-cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${YAML_CPP_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${YAML_CPP_LIBRARY_DIRS})
list(APPEND LIBRARIES ${YAML_CPP_LIBRARIES})

target_link_libraries(${PROJECT_NAME} ${LIBRARIES} converter Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)
add_test(${PROJECT_NAME} ${PROJECT_NAME})
