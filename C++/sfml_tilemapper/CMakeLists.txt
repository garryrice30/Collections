cmake_minimum_required(VERSION 3.16)

project(sfml_tilemapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Dependencies. TMXParser.h needs pugixml, zlib and zstd.
find_path(PUGIXML_INCLUDE_DIR pugixml.hpp)
find_library(PUGIXML_LIBRARY NAMES pugixml libpugixml)

find_path(ZLIB_INCLUDE_DIR zlib.h)
find_library(ZLIB_LIBRARY NAMES z libz)

find_path(ZSTD_INCLUDE_DIR zstd.h)
find_library(ZSTD_LIBRARY NAMES zstd libzstd)

if(NOT PUGIXML_INCLUDE_DIR OR NOT PUGIXML_LIBRARY)
    message(FATAL_ERROR "pugixml not found. Install it (Debian/Ubuntu: libpugixml-dev, "
                        "vcpkg: pugixml) or point CMAKE_PREFIX_PATH at your build of it.")
endif()
if(NOT ZLIB_INCLUDE_DIR OR NOT ZLIB_LIBRARY)
    message(FATAL_ERROR "zlib not found. Install it (Debian/Ubuntu: zlib1g-dev, vcpkg: zlib).")
endif()
if(NOT ZSTD_INCLUDE_DIR OR NOT ZSTD_LIBRARY)
    message(FATAL_ERROR "zstd not found. Install it (Debian/Ubuntu: libzstd-dev, vcpkg: zstd).")
endif()

add_executable(tilemapper_demo
    main.cpp
    TMXParserMap.cpp
    TMXParserData.cpp
    TMXParserTileset.cpp
    TMXParserTSX.cpp
    TMXParserDebug.cpp
)

target_include_directories(tilemapper_demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PUGIXML_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${ZSTD_INCLUDE_DIR}
)

target_link_libraries(tilemapper_demo PRIVATE
    ${PUGIXML_LIBRARY}
    ${ZLIB_LIBRARY}
    ${ZSTD_LIBRARY}
)

if(MSVC)
    target_compile_options(tilemapper_demo PRIVATE /W4)
else()
    target_compile_options(tilemapper_demo PRIVATE -Wall -Wextra)
endif()
