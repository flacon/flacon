 # BEGIN_COMMON_COPYRIGHT_HEADER
 # (c)LGPL2+
 #
 # Flacon - audio File Encoder
 # https://github.com/flacon/flacon
 #
 # Copyright: 2017
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

function(CREATE_PLIST_FILE _IN_FILE _OUT_FILE)
    configure_file(${_IN_FILE} ${_OUT_FILE} @ONLY)
    file(APPEND ${_OUT_FILE} "${name_tag}\n")
    file(APPEND ${_OUT_FILE} "${comment_tag}\n")
    file(APPEND ${_OUT_FILE} "${genericname_tag}\n")
endfunction()


# Homebrew has issues, fix it
macro(add_homebrew_qt_prefix_path)
    if (APPLE)
        file (GLOB dirs  /usr/local/Cellar/qt/*)
        list(APPEND CMAKE_PREFIX_PATH ${dirs})
    endif()
endmacro()
