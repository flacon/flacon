 # BEGIN_COMMON_COPYRIGHT_HEADER
 # (c)LGPL2+
 #
 # Flacon - audio File Encoder
 # https://github.com/flacon/flacon
 #
 # Copyright: 2012-2017
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

cmake_minimum_required(VERSION 3.0.2)
cmake_policy(SET CMP0028 NEW)
if (POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

set (CMAKE_CXX_STANDARD 11)
include("../cmake/cxx_standard.cmake")

project(converter)

set(HEADERS
    convertertypes.h
    wavheader.h
    decoder.h
    converter.h
    splitter.h
    encoder.h
    gain.h
    worker.h
    discpipline.h
    cuecreator.h
    coverimage.h
    sox.h
)

set(SOURCES
    convertertypes.cpp
    wavheader.cpp
    decoder.cpp
    converter.cpp
    splitter.cpp
    encoder.cpp
    gain.cpp
    worker.cpp
    discpipline.cpp
    cuecreator.cpp
    coverimage.cpp
    sox.cpp
)

# include("cmake/git_version.cmake")
# include("cmake/build_type.cmake")



#*******************************************
# include("cmake/macos.cmake")
# add_homebrew_qt_prefix_path()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED
    Core
    Widgets
)


# qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES} ${QT_LIBRARIES} Qt5::Widgets)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
