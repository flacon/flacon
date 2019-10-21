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

set(TRANSLATORS_INFO_FILE ${CMAKE_CURRENT_BINARY_DIR}/translators.info)

FUNCTION(create_translatorsinfo_qrc _qrcFile _DIR)
    get_filename_component(srcDir ${_DIR}/ ABSOLUTE)

    file(GLOB IN_FILES ${srcDir}/translators_*.info)
    file(WRITE ${TRANSLATORS_INFO_FILE} "")

    foreach(_file ${IN_FILES})
        get_filename_component(_name  ${_file} NAME)
        file(APPEND ${TRANSLATORS_INFO_FILE} "[${_name}]\n")
        file(READ ${_file} _content)
        file(APPEND ${TRANSLATORS_INFO_FILE} "${_content}\n")
    endforeach()

    configure_file(${srcDir}/translatorsinfo.qrc.in ${CMAKE_CURRENT_BINARY_DIR}/translatorsinfo.qrc)

    qt5_add_resources(__qrcFile ${CMAKE_CURRENT_BINARY_DIR}/translatorsinfo.qrc)

    set(${_qrcFile} ${__qrcFile} PARENT_SCOPE)
ENDFUNCTION()
