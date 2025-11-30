target_sources(
    gallop
    PUBLIC

    dllmain.cpp
    gui.cpp
    hook.cpp
    imgui_sink.cpp
    config.cpp
    mdb.cpp
    discord.cpp

    # Hooks
    hooks/models.cpp
    hooks/menu.cpp
)

target_include_directories(gallop PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
