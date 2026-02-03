# CPM manages the dependencies here
include(../cmake/CPM.cmake)

# Find and add all dependencies
CPMAddPackage("gh:nlohmann/json@3.12.0")
CPMAddPackage("gh:gabime/spdlog@1.16.0")
CPMAddPackage("gh:ToruNiina/toml11@4.4.0")
CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG 12.1.0
)
# This is an updated fork of the original discord rpc library
CPMAddPackage(
    NAME discord-rpc
    GITHUB_REPOSITORY harmonytf/discord-rpc
    GIT_TAG master
    OPTIONS "BUILD_EXAMPLES OFF"
)
CPMAddPackage(
    NAME SQLite3MultipleCiphers
    GITHUB_REPOSITORY utelle/SQLite3MultipleCiphers
    GIT_TAG main
    OPTIONS "SQLITE3MC_STATIC ON;SQLITE3MC_STATIC_RUNTIME_LINK OFF" 
)

set(SOURCE_FILES
    dllmain.cpp
    config.cpp
    discord.cpp
    gallop.hpp
    gui_integration.cpp
    hook.cpp
    mdb.cpp

    # Hooks
    hooks/menu.cpp
    hooks/models.cpp
)

# Compared to contemporary dll mods, this doesn't export as a version.dll hijack at all!
# It's best to use this with Hachimi and not directly as such, as that is detected by anticheat
# Despite this, this isn't a traditional library!
add_library(gallop SHARED ${SOURCE_FILES})

target_include_directories(gallop PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../thirdparty/sqlite_modern_cpp
)

target_link_libraries(gallop PRIVATE -static spdlog::spdlog sqlite3mc::sqlite3mc_static discord-rpc fmt::fmt nlohmann_json::nlohmann_json toml11::toml11)

target_compile_features(gallop PRIVATE cxx_std_20)
