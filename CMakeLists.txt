cmake_minimum_required(VERSION 3.10)
project(LagrangeCodec)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_C_COMPILE gcc)
set(CMAKE_CXX_COMPILER g++)

include(ProcessorCount)
include(FetchContent)

set(ENV{http_proxy} "http://127.0.0.1:7890")
set(ENV{https_proxy} "http://127.0.0.1:7890")

# Download ffmpeg source code

FetchContent_Declare(
        ffmpeg_fetch
        GIT_REPOSITORY "https://github.com/FFmpeg/FFmpeg.git"
        GIT_TAG "n4.1"
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(ffmpeg_fetch)
message(STATUS "FFmpeg source code will be downloaded to: ${ffmpeg_fetch_SOURCE_DIR}")

# Download silk source code

FetchContent_Declare(
        silk_fetch
        GIT_REPOSITORY "https://github.com/ploverlake/silk.git"
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(silk_fetch)
message(STATUS "SILK source code will be downloaded to: ${silk_fetch_SOURCE_DIR}")

# Prepare FFmpeg build

find_program(MAKE_EXECUTABLE
        NAMES gmake make nmake
        DOC "make executable"
        REQUIRED
)

find_package(PkgConfig)

if(PKG_CONFIG_FOUND)
    list(JOIN PKG_CONFIG_ARGN " " pkg_config_flags)
    set(pkgconfig_opts
            "--pkg-config=${PKG_CONFIG_EXECUTABLE}"
            "--pkg-config-flags=${pkg_config_flags}")
else()
    unset(pkgconfig_opts)
endif()

set(shared_opts
        --disable-shared
        --enable-static
)

set (size_opts --enable-small)
set (postfix_opts
        "--progs-suffix=${CMAKE_${BT}_POSTFIX}"
        "--build-suffix=${CMAKE_${BT}_POSTFIX}")
set(pic_opts --enable-pic)

# Configure FFmpeg build

ProcessorCount(numCores)

set(ffmpeg_build_dir ${CMAKE_BINARY_DIR}/ffmpeg_build)
set(ffmpeg_install_dir ${CMAKE_BINARY_DIR}/ffmpeg_install)
file(MAKE_DIRECTORY ${ffmpeg_build_dir})
file(MAKE_DIRECTORY ${ffmpeg_install_dir})

set(ffmpeg_configure_cmd
        ${ffmpeg_fetch_SOURCE_DIR}/configure

        --disable-programs

        ${shared_opts}
        ${pkgconfig_opts}
        ${size_opts}
        ${pic_opts}
        ${lto_opts}
        ${postfix_opts}

        "--strip=${CMAKE_STRIP}"
        "--extra-cflags=${CMAKE_C_FLAGS_${BT}}"
        "--optflags=${CMAKE_C_FLAGS_RELEASE}"
        "--cxx=${CMAKE_CXX_COMPILER}"
        "--extra-cxxflags=${CMAKE_CXX_FLAGS_${BT}}"
        "--objcc=${CMAKE_OBJC_COMPILER}"
        "--extra-objcflags=${CMAKE_OBJC_FLAGS_${BT}}"
)

set(ffmpeg_make_cmd
        ${MAKE_EXECUTABLE}
        -j${numCores}
)

# Build FFmpeg

execute_process(
        COMMAND ${ffmpeg_configure_cmd}
        WORKING_DIRECTORY ${ffmpeg_build_dir}
)

execute_process(
        COMMAND ${ffmpeg_make_cmd}
        WORKING_DIRECTORY ${ffmpeg_build_dir}
)

include_directories(
        ${ffmpeg_fetch_SOURCE_DIR}
        ${ffmpeg_build_dir}
)

# Including SILK

include_directories(
        ${silk_fetch_SOURCE_DIR}/src/SILK_SDK_SRC_ARM_v1.0.9/interface/
        ${silk_fetch_SOURCE_DIR}/src/SILK_SDK_SRC_ARM_v1.0.9/src/
)

file(GLOB SILK_SRC
        ${silk_fetch_SOURCE_DIR}/src/SILK_SDK_SRC_ARM_v1.0.9/src/*.c
)

# Shared Library Definition

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB SOURCE_FILES "./src/*.cpp")

add_library(${PROJECT_NAME} SHARED
        ${SOURCE_FILES}
        ${SILK_SRC}
)

target_link_libraries(${PROJECT_NAME}
        ${ffmpeg_build_dir}/libavcodec/libavcodec.a
        ${ffmpeg_build_dir}/libavformat/libavformat.a
        ${ffmpeg_build_dir}/libavutil/libavutil.a
        ${ffmpeg_build_dir}/libswresample/libswresample.a
        ${ffmpeg_build_dir}/libswscale/libswscale.a
)

if (APPLE)
    find_library(VIDEO_TOOLBOX VideoToolbox)
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(CORE_MEDIA CoreMedia)
    find_library(BZ2 bz2)
    find_library(CORE_VIDEO CoreVideo)
    find_library(ZLIB z)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    find_library(ICONV iconv)

    target_link_libraries(${PROJECT_NAME}
            ${VIDEO_TOOLBOX}
            ${CORE_FOUNDATION}
            ${CORE_MEDIA}
            ${BZ2}
            ${CORE_VIDEO}
            ${SECURITY}
            ${ZLIB}
            ${AUDIO_TOOLBOX}
            ${ICONV}
    )
elseif (WIN32)
elseif (LINUX)
endif ()