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

project(flacon)


set(MAJOR_VERSION 9)
set(MINOR_VERSION 0)
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


include("cmake/status_message.cmake")
status_message("${PROJECT_NAME} ver. ${FLACON_VERSION}")
status_message(" ")


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
    internet/dataprovider.h
    scanner.h
    patternexpander.h
    consoleout.h
    profiles.h
    audiofilematcher.h

    gui/icon.h
    gui/mainwindow.h
    gui/controls.h
    gui/trackview.h
    gui/trackviewmodel.h
    gui/trackviewdelegate.h
    gui/aboutdialog/aboutdialog.h
    gui/aboutdialog/translatorsinfo.h
    gui/coverdialog/coverdialog.h
    gui/coverdialog/asynclistwidgetitem.h
    gui/tageditor/tageditor.h
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
    internet/dataprovider.cpp
    scanner.cpp
    patternexpander.cpp
    consoleout.cpp
    profiles.cpp
    audiofilematcher.cpp

    gui/aboutdialog/aboutdialog.cpp
    gui/aboutdialog/translatorsinfo.cpp
    gui/coverdialog/coverdialog.cpp
    gui/coverdialog/asynclistwidgetitem.cpp
    gui/tageditor/tageditor.cpp
)

include(gui/module.cmake)
include(formats_in/module.cmake)
include(formats_out/module.cmake)

if (APPLE)
    set(SOURCES ${SOURCES}
        application_mac.mm
    )
endif()


set(LIBRARIES
)

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

set (CMAKE_CXX_STANDARD 14)
include("cmake/cxx_standard.cmake")
include("cmake/git_version.cmake")
include("cmake/build_type.cmake")

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
add_homebrew_qt_prefix_path()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED
    Core
    Widgets
    Network
    LinguistTools
)



qt5_add_translation(QM_FILES ${TS_FILES})


include(FindPkgConfig)
pkg_search_module(UCHARDET REQUIRED uchardet)

include(translations/translatorsinfo.cmake)
add_custom_target("translators")
add_custom_command(
    TARGET "translators"
    PRE_BUILD
    BYPRODUCTS ${TRANSLATORS_INFO_FILE}
    COMMAND create_translatorsinfo_qrc(TRANSLATORS_INFO_QRC translations)
)


set(LIBRARIES ${LIBRARIES} ${UCHARDET_LIBRARIES})
include_directories(${UCHARDET_INCLUDE_DIRS})
link_directories(${UCHARDET_LIBRARY_DIRS})


pkg_search_module(TAGLIB REQUIRED taglib)
set(LIBRARIES ${LIBRARIES} ${TAGLIB_LIBRARIES})
include_directories(${TAGLIB_INCLUDE_DIRS})
link_directories(${TAGLIB_LIBRARY_DIRS})


set(MAC_UPDATE_URL "https://flacon.github.io/download/feed.xml" CACHE STRING "The URL of updates appcast for MacOS")

if (APPLE)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    set(LIBRARIES ${LIBRARIES} ${COCOA_LIBRARY})
endif()

if ( MAC_BUNDLE )
    add_definitions(-DMAC_BUNDLE=Yes)
    add_definitions(-DMAC_UPDATER=Yes)

    set(HEADERS ${HEADERS}
        updater/updater.h
    )

    set(SOURCES ${SOURCES}
        updater/updater.mm
    )

    SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
    find_package(Sparkle REQUIRED)
    include_directories(${SPARKLE_INCLUDE_DIR})
    set(LIBRARIES ${LIBRARIES} ${SPARKLE_LIBRARY})


endif()

if (FLATPAK_BUNDLE)
	add_definitions(-DFLATPAK_BUNDLE=Yes)
endif()

add_subdirectory(converter)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${QM_FILES} ${QRC_SOURCES} ${ENGINES_CPP} ${ENGINES_H} ${RESOURCES} ${TRANSLATORS_INFO_QRC})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} converter ${QT_LIBRARIES} Qt5::Widgets Qt5::Network)


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
CREATE_PLIST_FILE(misc/Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
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


if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Flacon")
    install(TARGETS ${PROJECT_NAME} RUNTIME                 DESTINATION Flacon.app/Contents/MacOS)
    install(FILES   ${QM_FILES}                             DESTINATION Flacon.app/Contents/translations)
    install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/flacon.1.gz DESTINATION Flacon.app/Contents/Resources)
    install(FILES   ${CMAKE_CURRENT_BINARY_DIR}/Info.plist  DESTINATION Flacon.app/Contents)
    install(FILES   images/mainicon/Flacon.icns             DESTINATION Flacon.app/Contents/Resources)

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
