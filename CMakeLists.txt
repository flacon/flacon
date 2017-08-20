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

cmake_minimum_required(VERSION 2.6)

project(flacon)

set(MAJOR_VERSION 3)
set(MINOR_VERSION 1)
set(PATCH_VERSION 1)
set(FLACON_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})

add_definitions(-DFLACON_MAJOR_VERSION=\"${MAJOR_VERSION}\")
add_definitions(-DFLACON_MINOR_VERSION=\"${MINOR_VERSION}\")
add_definitions(-DFLACON_PATCH_VERSION=\"${PATCH_VERSION}\")
add_definitions(-DFLACON_VERSION=\"${FLACON_VERSION}\")


set(HEADERS
    tagset.h
    cue.h
    track.h
    disk.h
    project.h
    settings.h
    inputaudiofile.h
    internet/dataprovider.h
    scanner.h

    gui/mainwindow.h
    gui/controls.h
    gui/trackview.h
    gui/trackviewmodel.h
    gui/trackviewdelegate.h
    gui/configdialog/configdialog.h
    gui/aboutdialog/aboutdialog.h
    gui/aboutdialog/translatorsinfo.h
    gui/cuediskselectdialog/cuediskselectdialog.h

    converter/wavheader.h
    converter/decoder.h
    converter/converter.h
    converter/splitter.h
    converter/encoder.h
    converter/gain.h
    converter/worker.h
    converter/diskpipline.h
    converter/converterenv.h
    converter/cuecreator.h

    formats/format.h
    formats/outformat.h
    formats/aac.h
    formats/flac.h
    formats/mp3.h
    formats/ogg.h
    formats/wav.h
    formats/wv.h
    formats/opus.h
    formats/ape.h
    formats/tta.h
)

set(SOURCES
    tagset.cpp
    cue.cpp
    main.cpp
    track.cpp
    disk.cpp
    project.cpp
    settings.cpp
    inputaudiofile.cpp
    internet/dataprovider.cpp
    scanner.cpp

    gui/mainwindow.cpp
    gui/controls.cpp
    gui/trackview.cpp
    gui/trackviewmodel.cpp
    gui/trackviewdelegate.cpp
    gui/configdialog/configdialog.cpp
    gui/aboutdialog/aboutdialog.cpp
    gui/aboutdialog/translatorsinfo.cpp
    gui/cuediskselectdialog/cuediskselectdialog.cpp

    converter/wavheader.cpp
    converter/decoder.cpp
    converter/converter.cpp
    converter/splitter.cpp
    converter/encoder.cpp
    converter/gain.cpp
    converter/worker.cpp
    converter/diskpipline.cpp
    converter/cuecreator.cpp

    formats/format.cpp
    formats/outformat.cpp
    formats/aac.cpp
    formats/flac.cpp
    formats/mp3.cpp
    formats/ogg.cpp
    formats/wav.cpp
    formats/wv.cpp
    formats/opus.cpp
    formats/ape.cpp
    formats/tta.cpp
)

set (MOCS
    track.h
    disk.h
    project.h
    settings.h
    tagset.h
    internet/dataprovider.h
    scanner.h

    gui/mainwindow.h
    gui/controls.h
    gui/trackview.h
    gui/trackviewmodel.h
    gui/trackviewdelegate.h
    gui/configdialog/configdialog.h
    gui/aboutdialog/aboutdialog.h
    gui/cuediskselectdialog/cuediskselectdialog.h

    converter/decoder.h
    converter/converter.h
    converter/splitter.h
    converter/encoder.h
    converter/gain.h
    converter/worker.h
    converter/diskpipline.h

    formats/aac.h
    formats/flac.h
    formats/mp3.h
    formats/ogg.h
    formats/wv.h
    formats/opus.h
)

set (FORMS
    gui/mainwindow.ui
    gui/configdialog/configdialog.ui
    gui/aboutdialog/aboutdialog.ui
    gui/cuediskselectdialog/cuediskselectdialog.ui

    formats/flac_config.ui
    formats/aac_config.ui
    formats/mp3_config.ui
    formats/ogg_config.ui
    formats/wv_config.ui
    formats/opus_config.ui
)

set(LIBRARIES
)

# QRC files .................

set(RESOURCES
    images/images.qrc
    images/oxygen/icons.qrc
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

set (CMAKE_CXX_STANDARD 11)
include("tools.cmake")

############################################
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions (-Wall -pedantic -Wno-long-long -Werror=format-security)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    formats
    converter
    gui
    gui/configdialog
    gui/aboutdialog
)

#*******************************************

option(USE_QT5 "Force use the Qt5." $ENV{USE_QT5})
option(USE_QT4 "Force use the Qt4." $ENV{USE_QT4})

if((USE_QT4 AND USE_QT5) OR
   (NOT USE_QT4 AND NOT USE_QT5))
    find_package(Qt5Core QUIET)
    if (Qt5Core_FOUND)
        set(USE_QT4 OFF)
        set(USE_QT5 ON)
    else()
        set(USE_QT4 ON)
        set(USE_QT5 OFF)
    endif()
endif()


if(USE_QT4)
    set(QT_USE_QTNETWORK 1)
    find_package(Qt4 REQUIRED)
    include(${QT_USE_FILE})
    qt4_wrap_cpp(MOC_SOURCES ${MOCS})
    qt4_add_resources(QRC_SOURCES ${RESOURCES})
    qt4_wrap_ui(UI_HEADERS ${FORMS})
    qt4_add_translation(QM_FILES ${TS_FILES})

    status_message("Using Qt4, for building with Qt5 use -DUSE_QT5=Yes option.")
else()
    cmake_minimum_required(VERSION 2.8.3)
    find_package(Qt5Widgets REQUIRED QUIET)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    add_definitions(${Qt5Widgets_DEFINITIONS})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5Widgets_LIBRARIES})

    find_package(Qt5Network REQUIRED QUIET)
    include_directories(${Qt5Network_INCLUDE_DIRS})
    add_definitions(${Qt5Network_DEFINITIONS})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5Network_LIBRARIES})

    find_package(Qt5LinguistTools REQUIRED QUIET)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

    qt5_wrap_cpp(MOC_SOURCES ${MOCS})
    qt5_add_resources(QRC_SOURCES ${RESOURCES})
    qt5_wrap_ui(UI_HEADERS ${FORMS})
    qt5_add_translation(QM_FILES ${TS_FILES})

    status_message("Using Qt5, for building with Qt4 use -DUSE_QT4=Yes option.")
endif()

include(FindPkgConfig)
pkg_search_module(UCHARDET REQUIRED uchardet)

include(translations/translatorsinfo.cmake)
create_translatorsinfo_qrc(TRANSLATORS_INFO_QRC translations)
set(LIBRARIES ${LIBRARIES} ${UCHARDET_LIBRARIES})
include_directories(${UCHARDET_INCLUDE_DIRS})
link_directories(${UCHARDET_LIBRARY_DIRS})


add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QM_FILES} ${QRC_SOURCES} ${UI_HEADERS} ${ENGINES_CPP} ${ENGINES_H} ${TRANSLATORS_INFO_QRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} ${QT_LIBRARIES})


function(CREATE_DESKTOP_FILE _IN_FILE _OUT_FILE _TRANSLATIONS_PATTERN)
    file(GLOB ts_files ${_TRANSLATIONS_PATTERN})

    set(comment_tag "")
    set(name_tag "")
    set(genericname_tag "")
    foreach(f ${ts_files})
        file(READ ${f} contents)
        STRING(REGEX REPLACE ";" "\\\\;" contents "${contents}")
        STRING(REGEX REPLACE "\n" ";" contents "${contents}")

        foreach(l ${contents})
            if("${l}" MATCHES "^\\s*(Name)\\[.*\\]")
                set(name_tag "${name_tag}${l}\n")
            endif()

            if("${l}" MATCHES "^\\s*(Comment)\\[.*\\]")
                set(comment_tag "${comment_tag}${l}\n")
            endif()

            if("${l}" MATCHES "^\\s*(GenericName)\\[.*\\]")
                set(genericname_tag "${genericname_tag}${l}\n")
            endif()
        endforeach()
    endforeach()

    configure_file(${_IN_FILE} ${_OUT_FILE} @ONLY)
    file(APPEND ${_OUT_FILE} "${name_tag}\n")
    file(APPEND ${_OUT_FILE} "${comment_tag}\n")
    file(APPEND ${_OUT_FILE} "${genericname_tag}\n")
endfunction()

CREATE_DESKTOP_FILE(misc/${PROJECT_NAME}.desktop ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop translations/*.desktop)

install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop DESTINATION "share/applications")
install(FILES   misc/flacon-16x16.png   DESTINATION "share/icons/hicolor/16x16/apps" RENAME "flacon.png")
install(FILES   misc/flacon-32x32.png   DESTINATION "share/icons/hicolor/32x32/apps" RENAME "flacon.png")
install(FILES   misc/flacon-48x48.png   DESTINATION "share/icons/hicolor/48x48/apps" RENAME "flacon.png")
install(FILES   ${QM_FILES}             DESTINATION ${TRANSLATIONS_DIR})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)


addTests(tests)


# Man page **************************************
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/flacon.man.in
    ${CMAKE_CURRENT_BINARY_DIR}/flacon.1
    @ONLY
)

#add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/flacon.1.gz
add_custom_command(TARGET ${PROJECT_NAME}
    COMMAND gzip -c ${CMAKE_CURRENT_BINARY_DIR}/flacon.1 > ${CMAKE_CURRENT_BINARY_DIR}/flacon.1.gz
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/flacon.1.gz DESTINATION share/man/man1)

# Add make dist target **************************
if(NOT CPACK_PACKAGE_VERSION)
    SET(CPACK_PACKAGE_VERSION "${FLACON_VERSION}")
endif()

SET(CPACK_SOURCE_GENERATOR "TBZ2")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
if(EXISTS ${PROJECT_SOURCE_DIR}/.gitignore)
    file(READ .gitignore CPACK_IGNORE_FILES)
endif()

SET(CPACK_IGNORE_FILES
    ${CPACK_IGNORE_FILES}
    \\\\.git
    build
    CMakeLists.txt.user
    \\\\~
)

SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
INCLUDE(CPack)
# simulate autotools' "make dist"
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

show_status()
