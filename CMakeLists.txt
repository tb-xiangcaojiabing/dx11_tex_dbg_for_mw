cmake_minimum_required(VERSION 4.3.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(dx11_texture_debugger LANGUAGES CXX)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wno-strict-prototypes
        -Wno-unused-variable
        -Wno-unused-value
        -Wno-unused-parameter
        -Wno-unused-private-field
        -Wno-unused-function
    )
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/release")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
endif()

add_subdirectory(libs/minhook)

add_executable(dx11_tex_dbg)

target_sources(dx11_tex_dbg PRIVATE
    "./src/loader.cpp"
    "./res/res.rc"
)

target_sources(dx11_tex_dbg PRIVATE FILE_SET CXX_MODULES FILES
    "./src/console.cppm"
    "./src/cmdline.cppm"
)

add_library(dx11_texture_debugger SHARED
    "./src/dx11_tex_dbg.cpp"
)

target_link_libraries(dx11_tex_dbg PRIVATE gdiplus)

target_link_libraries(dx11_texture_debugger PRIVATE minhook d3d11 dxguid)


if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_options(dx11_tex_dbg PRIVATE
        "-Wl,/MANIFESTUAC:NO"
        "-Wl,/MANIFEST:EMBED"
        "-Wl,/MANIFESTINPUT:${CMAKE_SOURCE_DIR}/res/manifest.xml"
    )
endif()