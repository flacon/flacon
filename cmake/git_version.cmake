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

function(git_version GIT_BRANCH GIT_COMMIT_HASH)

    # Get the current working branch
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE branch
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    SET(${GIT_BRANCH} ${branch} PARENT_SCOPE)

    # Get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE hash
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    SET(${GIT_COMMIT_HASH} ${hash} PARENT_SCOPE)

endfunction()

git_version(GIT_BRANCH GIT_COMMIT_HASH)
if (NOT "${GIT_COMMIT_HASH}" STREQUAL "")
    add_definitions(-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\")
    add_definitions(-DGIT_BRANCH=\"${GIT_BRANCH}\")
endif()