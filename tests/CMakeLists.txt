 # BEGIN_COMMON_COPYRIGHT_HEADER
 # (c)LGPL2+
 #
 # Flacon - audio File Encoder
 # https://github.com/SokoloffA/flacon
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
cmake_minimum_required( VERSION 2.6 )


set(TEST_HEADERS
    testflacon.h
)

set(TEST_SOURCES
    testflacon.cpp
)

set(TEST_MOCS
    testflacon.h
)

foreach(FILE ${SOURCES})
    if(NOT FILE STREQUAL "main.cpp")
        set(TEST_SOURCES ${TEST_SOURCES} "../${FILE}")
    endif()
endforeach()


foreach(FILE ${HEADERS})
    set(TEST_HEADERS ${TEST_HEADERS} "../${FILE}")
endforeach()


foreach(FILE ${MOCS})
    set(TEST_MOCS ${TEST_MOCS} "../${FILE}")
endforeach()


add_definitions(-DTEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\")

set(TEST_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/output/")
file(MAKE_DIRECTORY ${TEST_OUT_DIR})
add_definitions(-DTEST_OUT_DIR=\"${TEST_OUT_DIR}\")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(QT_USE_QTTEST true)

add_definitions(-Wall)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
qt4_wrap_cpp(TEST_MOC_SOURCES ${TEST_MOCS})


add_executable(${PROJECT_NAME} ${TEST_HEADERS} ${TEST_SOURCES} ${TEST_MOC_SOURCES})
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES}  ${LIBRARIES})

#add_dependencies(${PROJECT_NAME} razorqt)
#install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

