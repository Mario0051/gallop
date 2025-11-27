cmake_minimum_required(VERSION 3.20)

project(gallop)

add_compile_definitions(UNICODE)
add_compile_definitions(_UNICODE)

# CPM manages the dependencies here
include(cmake/CPM.cmake)

# Target is always set to MinGW-w64
# This is a hachimi module, after all
set(CMAKE_SYSTEM Windows)
set(TARGET "x86_64-w64-mingw32")
set(CMAKE_C_COMPILER_TARGET ${TARGET})
set(CMAKE_CXX_COMPILER_TARGET ${TARGET})
set(CMAKE_SYSTEM_NAME "Windows")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Compile flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Find and add all dependencies
CPMAddPackage("gh:nlohmann/json@3.12.0")
CPMAddPackage("gh:cursey/safetyhook@0.6.9")
CPMAddPackage("gh:TsudaKageyu/minhook@1.3.4")
CPMAddPackage("gh:gabime/spdlog@1.16.0")
CPMAddPackage("gh:ToruNiina/toml11@4.4.0")
CPMAddPackage(
    NAME SQLite3MultipleCiphers
    GITHUB_REPOSITORY utelle/SQLite3MultipleCiphers
    GIT_TAG main
    OPTIONS "SQLITE3MC_STATIC ON; SQLITE3MC_STATIC_RUNTIME_LINK ON"
)
# OpenGL manages the GUI portion
find_package(OpenGL REQUIRED)

# Compared to contemporary dll mods, this doesn't export as a version.dll hijack at all!
# It's best to use this with Hachimi and not directly as such, as that is detected by anticheat
# Despite this, this isn't a traditional library!
add_library(gallop SHARED)

add_subdirectory(thirdparty)
add_subdirectory(src)
target_link_libraries(gallop PRIVATE -static -ldwmapi OpenGL::GL nlohmann_json::nlohmann_json toml11::toml11 safetyhook::safetyhook minhook spdlog::spdlog sqlite3mc::sqlite3mc_static)
