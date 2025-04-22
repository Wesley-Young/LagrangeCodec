cmake_minimum_required(VERSION 3.24)
project(LagrangeCodec)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (LINUX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wl,-Bsymbolic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wl,-Bsymbolic")
endif()
if(MSVC)
    message(STATUS "MSVC compiler detected")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

include(FetchContent)
FetchContent_Declare(
        silk_fetch
        GIT_REPOSITORY "https://github.com/kn007/silk-v3-decoder.git"
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(silk_fetch)
message(STATUS "SILK source code will be downloaded to: ${silk_fetch_SOURCE_DIR}")

find_package(FFMPEG REQUIRED)

find_package(ZLIB REQUIRED)

file(GLOB SOURCES "src/*.cpp")
add_library(LagrangeCodec SHARED ${SOURCES})

target_include_directories(LagrangeCodec PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_include_directories(LagrangeCodec PRIVATE
        ${silk_fetch_SOURCE_DIR}/silk/interface
        ${silk_fetch_SOURCE_DIR}/silk/include
        ${silk_fetch_SOURCE_DIR}/silk/src
)
file(GLOB SILK_SRC_FILES
        ${silk_fetch_SOURCE_DIR}/silk/src/*.c
        ${silk_fetch_SOURCE_DIR}/silk/src/*.S
)
target_sources(LagrangeCodec PRIVATE ${SILK_SRC_FILES})

target_include_directories(LagrangeCodec PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(LagrangeCodec PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(LagrangeCodec PRIVATE ${FFMPEG_LIBRARIES} ZLIB::ZLIB)

enable_testing()
find_package(GTest CONFIG REQUIRED)
add_subdirectory(tests)
