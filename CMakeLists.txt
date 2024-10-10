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

cmake_minimum_required(VERSION 3.16.0)
cmake_policy(SET CMP0028 NEW)
if (POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

project(flacon)


set(MAJOR_VERSION 11)
set(MINOR_VERSION 4)
set(PATCH_VERSION 0)
#set(BETA_VERSION beta1)

set(FLACON_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
if (BETA_VERSION)
    set(FLACON_VERSION ${FLACON_VERSION}-${BETA_VERSION})
endif()

add_definitions(-DFLACON_MAJOR_VERSION=\"${MAJOR_VERSION}\")
add_definitions(-DFLACON_MINOR_VERSION=\"${MINOR_VERSION}\")
add_definitions(-DFLACON_PATCH_VERSION=\"${PATCH_VERSION}\")
add_definitions(-DFLACON_VERSION=\"${FLACON_VERSION}\")

set (CMAKE_CXX_STANDARD 17)


SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include("cmake/status_message.cmake")
include("cmake/cxx_standard.cmake")
include("cmake/git_version.cmake")
include("cmake/build_type.cmake")

status_message("${PROJECT_NAME} ver. ${FLACON_VERSION}")
status_message(" ")

if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
    option(MAC_BUNDLE "Create .app bundle for macOS" OFF)
endif()

if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    option(FLATPAK_BUNDLE  "Create Flatpak bundle"  OFF)
    option(APPIMAGE_BUNDLE "Create AppImage bundle" OFF)
endif()


if ( MAC_BUNDLE )
    add_definitions(-DMAC_BUNDLE)
    add_definitions(-DBUNDLED_PROGRAMS)
    add_definitions(-DMAC_UPDATER)

elseif ( FLATPAK_BUNDLE )
    add_definitions(-DFLATPAK_BUNDLE)
    add_definitions(-DBUNDLED_PROGRAMS)
    add_definitions(-DDISABLE_TMP_DIR)

elseif ( APPIMAGE_BUNDLE )
    add_definitions(-DAPPIMAGE_BUNDLE)
    add_definitions(-DBUNDLED_PROGRAMS)

endif()


set(HEADERS
    application.h
    types.h
    tags.h
    cue.h
    cuedata.h
    track.h
    disc.h
    project.h
    settings.h
    inputaudiofile.h
    scanner.h
    patternexpander.h
    consoleout.h
    profiles.h
    audiofilematcher.h
    sync.h
    extprogram.h
    uchardetect.h
    textcodec.h
    debug.h

    gui/icon.h
    gui/mainwindow.h
    gui/controls.h
    gui/trackview.h
    gui/trackviewmodel.h
    gui/trackviewdelegate.h
    gui/aboutdialog/aboutdialog.h
    gui/coverdialog/coverdialog.h
    gui/coverdialog/asynclistwidgetitem.h
 )

set(SOURCES
    application.cpp
    types.cpp
    tags.cpp
    cue.cpp
    cuedata.cpp
    main.cpp
    track.cpp
    disc.cpp
    project.cpp
    settings.cpp
    inputaudiofile.cpp
    scanner.cpp
    patternexpander.cpp
    consoleout.cpp
    profiles.cpp
    audiofilematcher.cpp
    extprogram.cpp
    uchardetect.cpp
    textcodec.cpp
    debug.cpp

    gui/aboutdialog/aboutdialog.cpp
    gui/coverdialog/coverdialog.cpp
    gui/coverdialog/asynclistwidgetitem.cpp
)

if (APPLE)
    set(SOURCES ${SOURCES} application_mac.mm )
endif()


include(internet/module.cmake)
include(gui/module.cmake)
include(formats_in/module.cmake)
include(formats_out/module.cmake)
include(qtbackports/module.cmake)
include(validator/module.cmake)

set(LIBRARIES)

# QRC files .................

set(RESOURCES
    images/images.qrc
    images/icons.qrc
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


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen
    formats
    converter
    gui
    gui/configdialog
    gui/aboutdialog
)

#*******************************************
include("cmake/macos.cmake")

option(USE_QT5 "Try to use the Qt5." ON)
option(USE_QT6 "Try to use the Qt6." OFF)

if (USE_QT5 AND USE_QT6)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
elseif (USE_QT6)
    find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
elseif (USE_QT5)
    find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)
endif()
message(STATUS "Using Qt version ${QT_VERSION}")


set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED
    Core
    Widgets
    Network
    LinguistTools
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()


include(FindPkgConfig)
pkg_search_module(UCHARDET REQUIRED uchardet)
message(STATUS "Using uchardet version: ${UCHARDET_VERSION}")

set(LIBRARIES ${LIBRARIES} ${UCHARDET_LIBRARIES})
include_directories(${UCHARDET_INCLUDE_DIRS})
link_directories(${UCHARDET_LIBRARY_DIRS})


pkg_search_module(TAGLIB REQUIRED taglib)
set(LIBRARIES ${LIBRARIES} ${TAGLIB_LIBRARIES})
include_directories(${TAGLIB_INCLUDE_DIRS})
link_directories(${TAGLIB_LIBRARY_DIRS})
message(STATUS "Using taglib version: ${TAGLIB_VERSION}")


find_package(Iconv REQUIRED)
set(LIBRARIES ${LIBRARIES} ${Iconv_LIBRARIES})


if (APPLE)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    set(LIBRARIES ${LIBRARIES} ${COCOA_LIBRARY})
endif()


if ( MAC_BUNDLE )
    set(HEADERS ${HEADERS} updater/updater.h )
    set(SOURCES ${SOURCES} updater/updater.mm )


    set(MAC_UPDATE_URL "https://flacon.github.io/download/feed.xml" CACHE STRING "The URL of updates appcast for MacOS")
    CREATE_PLIST_FILE(misc/Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)

    find_package(Sparkle REQUIRED)
    include_directories(${SPARKLE_INCLUDE_DIR})
    set(LIBRARIES ${LIBRARIES} ${SPARKLE_LIBRARY})

endif()


add_subdirectory(converter)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${QM_FILES} ${QRC_SOURCES} ${ENGINES_CPP} ${ENGINES_H} ${RESOURCES} ${TRANSLATORS_INFO_QRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} converter Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)


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

function(CREATE_FLATPAK_METAINFO_FILE _IN_FILE _OUT_FILE)
    string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
    configure_file(${_IN_FILE} ${_OUT_FILE} @ONLY)
endfunction()

CREATE_DESKTOP_FILE(misc/${PROJECT_NAME}.desktop ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop translations/*.desktop)
CREATE_FLATPAK_METAINFO_FILE(misc/com.github.Flacon.metainfo.xml.in ${CMAKE_CURRENT_BINARY_DIR}/com.github.Flacon.metainfo.xml)

# Man page **************************************
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/flacon.man.in
    ${CMAKE_CURRENT_BINARY_DIR}/flacon.1
    @ONLY
)

add_custom_command(TARGET ${PROJECT_NAME}
    COMMAND gzip -c -n ${CMAKE_CURRENT_BINARY_DIR}/flacon.1 > ${CMAKE_CURRENT_BINARY_DIR}/flacon.1.gz
)

string(REPLACE "/lib/cmake/Qt5Core" "" QT_DIR "${Qt5Core_DIR}")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Flacon")
    install(TARGETS ${PROJECT_NAME} RUNTIME                 DESTINATION Flacon.app/Contents/MacOS)
    install(FILES   ${QM_FILES}                             DESTINATION Flacon.app/Contents/translations)
    install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/flacon.1.gz DESTINATION Flacon.app/Contents/Resources)
    install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/Info.plist  DESTINATION Flacon.app/Contents)
    install(FILES   images/mainicon/Flacon.icns             DESTINATION Flacon.app/Contents/Resources)

    set(QT_PLUGINS
        "bearer/libqgenericbearer.dylib"
        "iconengines/libqsvgicon.dylib"
        "imageformats/libqgif.dylib"
        "imageformats/libqicns.dylib"
        "imageformats/libqico.dylib"
        "imageformats/libqjpeg.dylib"
        "platforms/libqcocoa.dylib"
        "styles/libqmacstyle.dylib"
    )

    foreach(plugin ${QT_PLUGINS})
        get_filename_component(dest_dir "Flacon.app/Contents/PlugIns/${plugin}" DIRECTORY)
        install(FILES "${QT_DIR}/plugins/${plugin}" DESTINATION "${dest_dir}")
    endforeach()


    set(BUNDLE_DIR "${CMAKE_BINARY_DIR}/Flacon.app")
    set(DIRS ${CMAKE_PREFIX_PATH} $ENV{HOME}/Library/Frameworks)
    configure_file(cmake/macdelopy.cmake.in "${CMAKE_BINARY_DIR}/macdelopy.cmake" @ONLY)
    install(SCRIPT "${CMAKE_BINARY_DIR}/macdelopy.cmake")

else()
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
    install(FILES   ${QM_FILES}             DESTINATION ${TRANSLATIONS_DIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/flacon.1.gz DESTINATION share/man/man1)

    install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop        DESTINATION "share/applications")
    install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/com.github.Flacon.metainfo.xml DESTINATION "share/metainfo")
    install(FILES   images/mainicon/flacon-16x16.png   DESTINATION "share/icons/hicolor/16x16/apps"    RENAME "flacon.png")
    install(FILES   images/mainicon/flacon-32x32.png   DESTINATION "share/icons/hicolor/32x32/apps"    RENAME "flacon.png")
    install(FILES   images/mainicon/flacon-48x48.png   DESTINATION "share/icons/hicolor/48x48/apps"    RENAME "flacon.png")
    install(FILES   images/mainicon/flacon-64x64.png   DESTINATION "share/icons/hicolor/64x64/apps"    RENAME "flacon.png")
    install(FILES   images/mainicon/flacon-128x128.png DESTINATION "share/icons/hicolor/128x128/apps"  RENAME "flacon.png")
    install(FILES   images/mainicon/flacon-256x256.png DESTINATION "share/icons/hicolor/256x256/apps"  RENAME "flacon.png")
    install(FILES   images/mainicon/flacon-512x512.png DESTINATION "share/icons/hicolor/512x512/apps"  RENAME "flacon.png")
    install(FILES   images/mainicon/flacon.svg         DESTINATION "share/icons/hicolor/scalable/apps" RENAME "flacon.svg")

endif()

include("cmake/tests.cmake")
add_tests(tests)


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
