# imgui
target_sources(
    gallop
    PUBLIC

    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
    # Win32 + OpenGL backend
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backend/imgui_impl_win32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backend/imgui_impl_opengl3.cpp
)

target_include_directories(gallop PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

# aetherim (il2cpp library)
target_include_directories(gallop PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/aetherim")

# sqlite_modern_cpp
target_include_directories(gallop PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/sqlite_modern_cpp")

# nameof
target_include_directories(gallop PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/nameof")
