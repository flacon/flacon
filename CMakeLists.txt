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

cmake_minimum_required(VERSION 2.6)

project(flacon)

set(FLACON_MAJOR_VERSION 0)
set(FLACON_MINOR_VERSION 8)
set(FLACON_PATCH_VERSION 99)
set(FLACON_VERSION ${FLACON_MAJOR_VERSION}.${FLACON_MINOR_VERSION}.${FLACON_PATCH_VERSION})

add_definitions(-DFLACON_MAJOR_VERSION=\"${MAJOR_VERSION}\")
add_definitions(-DFLACON_MINOR_VERSION=\"${MINOR_VERSION}\")
add_definitions(-DFLACON_PATCH_VERSION=\"${PATCH_VERSION}\")
add_definitions(-DFLACON_VERSION=\"${FLACON_VERSION}\")


set(HEADERS
    tagset.h
    disk.h
    project.h
    settings.h
    inputaudiofile.h
    internet/dataprovider.h

    gui/mainwindow.h
    gui/controls.h
    gui/trackview.h
    gui/trackviewmodel.h
    gui/trackviewdelegate.h
    gui/configdialog/configdialog.h
    gui/aboutdialog/aboutdialog.h
    gui/aboutdialog/translatorsinfo.h


    converter/converterthread.h
    converter/converter.h
    converter/splitter.h
    converter/encoder.h
    converter/gain.h

    outformats/outformat.h
    outformats/aac.h
    outformats/wav.h
    outformats/flac.h
    outformats/mp3.h
    outformats/ogg.h
)

set(SOURCES
    tagset.cpp
    main.cpp
    disk.cpp
    project.cpp
    settings.cpp
    inputaudiofile.cpp
    internet/dataprovider.cpp

    gui/mainwindow.cpp
    gui/controls.cpp
    gui/trackview.cpp
    gui/trackviewmodel.cpp
    gui/trackviewdelegate.cpp
    gui/configdialog/configdialog.cpp
    gui/aboutdialog/aboutdialog.cpp
    gui/aboutdialog/translatorsinfo.cpp

    converter/converterthread.cpp
    converter/converter.cpp
    converter/splitter.cpp
    converter/encoder.cpp
    converter/gain.cpp

    outformats/outformat.cpp
    outformats/aac.cpp
    outformats/wav.cpp
    outformats/flac.cpp
    outformats/mp3.cpp
    outformats/ogg.cpp
)

set (MOCS
    disk.h
    project.h
    settings.h
    tagset.h

    internet/dataprovider.h

    gui/mainwindow.h
    gui/controls.h
    gui/trackview.h
    gui/trackviewmodel.h
    gui/trackviewdelegate.h
    gui/configdialog/configdialog.h
    gui/aboutdialog/aboutdialog.h

    converter/converterthread.h
    converter/converter.h
    converter/splitter.h
    converter/encoder.h
    converter/gain.h

    outformats/aac.h
    outformats/wav.h
    outformats/flac.h
    outformats/mp3.h
    outformats/ogg.h
)

set (FORMS
    gui/mainwindow.ui
    gui/configdialog/configdialog.ui
    gui/aboutdialog/aboutdialog.ui

    outformats/flac_config.ui
    outformats/aac_config.ui
    outformats/mp3_config.ui
    outformats/ogg_config.ui
)

set(LIBRARIES
)

# QRC files .................

set(RESOURCES
    images/images.qrc
)

# add translations ...
file (GLOB TS_FILES 
    translations/flacon*.ts
)

set(DATA_DIR 
    ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}
)

set(TRANSLATIONS_DIR
    "${DATA_DIR}/translations"
)
add_definitions(-DTRANSLATIONS_DIR=\"${TRANSLATIONS_DIR}\")


############################################
if(CMAKE_COMPILER_IS_GNUCXX)
      add_definitions (-Wall -pedantic -Wno-long-long)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    outformats
    converter
    gui
    gui/configdialog
    gui/aboutdialog
)

#*******************************************

set(QT_USE_QTNETWORK 1)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_SOURCES ${MOCS})
qt4_add_resources(QRC_SOURCES ${RESOURCES})
qt4_wrap_ui(UI_HEADERS ${FORMS})
qt4_add_translation(QM_FILES ${TS_FILES})

include(FindPkgConfig)
pkg_search_module(UCHARDET REQUIRED uchardet)

include(translations/translatorsinfo.cmake)
create_translatorsinfo_qrc(TRANSLATORS_INFO_QRC translations)
set(LIBRARIES ${LIBRARIES} ${UCHARDET_LIBRARIES})
include_directories(${UCHARDET_INCLUDE_DIRS})


add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QM_FILES} ${QRC_SOURCES} ${UI_HEADERS} ${ENGINES_CPP} ${ENGINES_H} ${TRANSLATORS_INFO_QRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} ${QT_LIBRARIES})

install(FILES   misc/${PROJECT_NAME}.desktop DESTINATION "share/applications")
install(FILES   misc/flacon-16x16.png   DESTINATION "share/icons/hicolor/16x16/apps")
install(FILES   misc/flacon-32x32.png   DESTINATION "share/icons/hicolor/32x32/apps")
install(FILES   misc/flacon-48x48.png   DESTINATION "share/icons/hicolor/48x48/apps")
install(FILES   ${QM_FILES}             DESTINATION ${TRANSLATIONS_DIR})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)


if(BUILD_TESTS STREQUAL "Yes")
    add_subdirectory(tests)
else()
    message(STATUS "")
    message(STATUS "For building tests use -DBUILD_TESTS=Yes option.")
    message(STATUS "")
endif()


