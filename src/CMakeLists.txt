add_executable(gallop_gui WIN32 
    gui_main.cpp 
    gui.cpp 
    imgui_sink.cpp

    config.cpp
    mdb.cpp

    ../thirdparty/imgui/imgui.cpp
    ../thirdparty/imgui/imgui_draw.cpp
    ../thirdparty/imgui/imgui_tables.cpp
    ../thirdparty/imgui/imgui_widgets.cpp
    ../thirdparty/imgui/backend/imgui_impl_win32.cpp
    ../thirdparty/imgui/backend/imgui_impl_opengl3.cpp
)

target_include_directories(gallop_gui PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/aetherim"
    "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/sqlite_modern_cpp"
)

target_link_options(gallop_gui PRIVATE -static)

target_link_libraries(gallop_gui PRIVATE 
    spdlog::spdlog 
    opengl32 
    dwmapi
    nlohmann_json::nlohmann_json
    safetyhook
    minhook
    sqlite3mc_static
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gui_binary.h
    COMMAND strip -s $<TARGET_FILE:gallop_gui>
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gallop_gui> ${CMAKE_CURRENT_BINARY_DIR}/gallop_gui_embed.exe
    COMMAND xxd -i gallop_gui_embed.exe > ${CMAKE_CURRENT_BINARY_DIR}/gui_binary.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS gallop_gui
    COMMENT "Stripping and embedding GUI executable..."
)

add_custom_target(generate_gui_header DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/gui_binary.h)

target_sources(
    gallop
    PUBLIC

    dllmain.cpp
    hook.cpp
    config.cpp
    mdb.cpp

    # Hooks
    hooks/models.cpp
)

add_dependencies(gallop generate_gui_header)

target_include_directories(gallop PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(gallop PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
