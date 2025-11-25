target_sources(
    gallop
    PUBLIC

    dllmain.cpp
    gui.cpp
    hook.cpp
    imgui_sink.cpp
    config.cpp

    # Hooks
    hooks/models.cpp
)

target_include_directories(gallop PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
