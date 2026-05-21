cmake_minimum_required(VERSION 3.12)
project(mediainfo_bin)

include(ExternalProject)

set(MEDIAINFO_TAG v25.03)

set(ZENLIB_STATIC        "ZenLib/Project/GNU/Library/.libs/libzen.a")
set(MEDIAINFOLIB_STATIC  "MediaInfoLib/Project/GNU/Library/.libs/libmediainfo.a")
set(MEDIAINFO_OUT        "${CMAKE_BINARY_DIR}/tests/mediainfo")

# ==========================================
# 1. ZenLib
# ==========================================
ExternalProject_Add(
    zenlib
    SOURCE_DIR        "ZenLib"
    GIT_REPOSITORY    "https://github.com/MediaArea/ZenLib.git"
    GIT_TAG           "v0.4.41"
    GIT_SHALLOW       ON

    EXCLUDE_FROM_ALL  ON
    BUILD_IN_SOURCE   ON
    CONFIGURE_COMMAND cd "Project/GNU/Library" && ./autogen.sh > /dev/null 2>&1 && ./configure --quiet --enable-static --disable-shared --enable-silent-rules
    BUILD_COMMAND     cd "Project/GNU/Library" && make --silent -j4
    INSTALL_COMMAND   ""

    BUILD_BYPRODUCTS  "${ZENLIB_STATIC}"
)


# ==========================================
# 2. MediaInfoLib
# ==========================================
ExternalProject_Add(
    mediainfolib
    SOURCE_DIR        "MediaInfoLib"
    GIT_REPOSITORY    "https://github.com/MediaArea/MediaInfoLib.git"
    GIT_TAG           "${MEDIAINFO_TAG}"
    GIT_SHALLOW       ON

    EXCLUDE_FROM_ALL  ON
    BUILD_IN_SOURCE   ON
    CONFIGURE_COMMAND cd "Project/GNU/Library" && ./autogen.sh > /dev/null 2>&1 && ./configure --quiet --enable-static --enable-silent-rules
    BUILD_COMMAND     cd "Project/GNU/Library" && make --silent -j4
    INSTALL_COMMAND   ""

    BUILD_BYPRODUCTS  "${MEDIAINFOLIB_STATIC}"
    DEPENDS           zenlib
)


# ==========================================
# 3. MediaInfo CLI
# ==========================================
ExternalProject_Add(
    mediainfo_cli
    SOURCE_DIR        "MediaInfo"
    GIT_REPOSITORY    "https://github.com/MediaArea/MediaInfo.git"
    GIT_TAG           "${MEDIAINFO_TAG}"
    GIT_SHALLOW       ON

    EXCLUDE_FROM_ALL  ON
    BUILD_IN_SOURCE   ON
    CONFIGURE_COMMAND cd "Project/GNU/CLI" && ./autogen.sh > /dev/null 2>&1 && ./configure --quiet --enable-staticlibs --enable-silent-rules
    BUILD_COMMAND     cd "Project/GNU/CLI" && make --silent -j4

    INSTALL_COMMAND   ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/tests"
          COMMAND     ${CMAKE_COMMAND} -E copy "Project/GNU/CLI/.libs/mediainfo" "${MEDIAINFO_OUT}"

    DEPENDS           mediainfolib
)

add_custom_command(
    OUTPUT "${MEDIAINFO_OUT}"
    COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target mediainfo_cli
    COMMENT "Verifying MediaInfo binary status on build step..."
)

add_custom_target(mediainfo_bin ALL DEPENDS "${MEDIAINFO_OUT}")

