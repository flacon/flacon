project(mediainfo_bin)

include (ExternalProject)

set(MEDIAINFO_TAG v25.03)

set(ZENLIB_OUT     "ZenLib/Project/GNU/Library/.libs/libzen.a")
set(MEDIAINFOLIB_OUT "MediaInfoLib/Project/GNU/Library/.libs/libmediainfo.a")
set(MEDIAINFO_OUT  "${CMAKE_BINARY_DIR}/tests/mediainfo")

# ZenLib
add_custom_command(
    OUTPUT  ${ZENLIB_OUT}
    COMMAND git clone --depth 1 --branch v0.4.41 https://github.com/MediaArea/ZenLib.git 2>/dev/null
            || true
    COMMAND cd "ZenLib/Project/GNU/Library" &&
            ./autogen.sh > /dev/null 2>&1 &&
            ./configure --quiet --enable-static --enable-silent-rules &&
            make --silent
    COMMENT "Building ZenLib"
)

# MediaInfoLib
add_custom_command(
    OUTPUT  ${MEDIAINFOLIB_OUT}
    DEPENDS ${ZENLIB_OUT}
    COMMAND git clone --depth 1 --branch ${MEDIAINFO_TAG} https://github.com/MediaArea/MediaInfoLib.git 2>/dev/null
            || true
    COMMAND cd "MediaInfoLib/Project/GNU/Library" &&
            ./autogen.sh > /dev/null 2>&1 &&
            ./configure --quiet --enable-static --enable-silent-rules &&
            make --silent
    COMMENT "Building MediaInfoLib"
)

# MediaInfo
add_custom_command(
    OUTPUT  ${MEDIAINFO_OUT}
    DEPENDS ${ZENLIB_OUT} ${MEDIAINFOLIB_OUT}
    COMMAND git clone --depth 1 --branch ${MEDIAINFO_TAG} https://github.com/MediaArea/MediaInfo.git 2>/dev/null
            || true
    COMMAND cd "MediaInfo/Project/GNU/CLI" &&
            ./autogen.sh > /dev/null 2>&1 &&
            ./configure --quiet --enable-staticlibs --enable-silent-rules &&
            make --silent
    COMMAND install "MediaInfo/Project/GNU/CLI/.libs/mediainfo" "${MEDIAINFO_OUT}"
    COMMENT "Building MediaInfo CLI"
)

add_custom_target(mediainfo_bin ALL DEPENDS ${MEDIAINFO_OUT})
