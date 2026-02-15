#pragma once
#include <cstdint>
#include <cstddef>

struct Il2CppImage;
struct Il2CppClass;
struct Il2CppObject;
struct Il2CppTypeEnum;
struct Il2CppMethod;
struct Il2CppThread;
struct FieldInfo;
struct MethodInfo;
struct Il2CppArray;

struct Hachimi;
struct Interceptor;

enum class InitResult : int32_t {
    Error = 0,
    Ok = 1
};

constexpr int32_t LOG_LEVEL_ERROR = 1;
constexpr int32_t LOG_LEVEL_WARN = 2;
constexpr int32_t LOG_LEVEL_INFO = 3;
constexpr int32_t LOG_LEVEL_DEBUG = 4;
constexpr int32_t LOG_LEVEL_TRACE = 5;

typedef void (*GuiMenuCallback)(void* userdata);
typedef void (*GuiMenuSectionCallback)(void* ui, void* userdata);
typedef void (*GuiUiCallback)(void* ui, void* userdata);

struct HachimiVtable {
    const Hachimi* (*hachimi_instance)();
    const Interceptor* (*hachimi_get_interceptor)(const Hachimi* this_ptr);

    void* (*interceptor_hook)(const Interceptor* this_ptr, void* orig_addr, void* hook_addr);
    void* (*interceptor_hook_vtable)(const Interceptor* this_ptr, void** vtable, size_t index, void* hook_addr);
    void* (*interceptor_get_trampoline_addr)(const Interceptor* this_ptr, void* hook_addr);
    void* (*interceptor_unhook)(const Interceptor* this_ptr, void* hook_addr);

    void* (*il2cpp_resolve_symbol)(const char* name);
    const Il2CppImage* (*il2cpp_get_assembly_image)(const char* assembly_name);
    Il2CppClass* (*il2cpp_get_class)(const Il2CppImage* image, const char* namespaze, const char* class_name);

    const MethodInfo* (*il2cpp_get_method)(Il2CppClass* klass, const char* name, int32_t args_count);
    const MethodInfo* (*il2cpp_get_method_overload)(Il2CppClass* klass, const char* name, const Il2CppTypeEnum* params, size_t param_count);
    void* (*il2cpp_get_method_addr)(Il2CppClass* klass, const char* name, int32_t args_count);
    void* (*il2cpp_get_method_overload_addr)(Il2CppClass* klass, const char* name, const Il2CppTypeEnum* params, size_t param_count);
    const MethodInfo* (*il2cpp_get_method_cached)(Il2CppClass* klass, const char* name, int32_t args_count);
    void* (*il2cpp_get_method_addr_cached)(Il2CppClass* klass, const char* name, int32_t args_count);
    Il2CppClass* (*il2cpp_find_nested_class)(Il2CppClass* klass, const char* name);

    FieldInfo* (*il2cpp_get_field_from_name)(Il2CppClass* klass, const char* name);
    void (*il2cpp_get_field_value)(Il2CppObject* obj, FieldInfo* field, void* out_value);
    void (*il2cpp_set_field_value)(Il2CppObject* obj, FieldInfo* field, const void* value);
    void (*il2cpp_get_static_field_value)(FieldInfo* field, void* out_value);
    void (*il2cpp_set_static_field_value)(FieldInfo* field, const void* value);
    void* (*il2cpp_unbox)(Il2CppObject* obj);

    Il2CppThread* (*il2cpp_get_main_thread)();
    Il2CppThread** (*il2cpp_get_attached_threads)(size_t* out_size);
    void (*il2cpp_schedule_on_thread)(Il2CppThread* thread, void (*callback)());
    Il2CppArray* (*il2cpp_create_array)(Il2CppClass* element_type, size_t length);
    Il2CppObject* (*il2cpp_get_singleton_like_instance)(Il2CppClass* klass);

    void (*log)(int32_t level, const char* target, const char* message);

    bool (*gui_register_menu_item)(const char* label, GuiMenuCallback callback, void* userdata);
    bool (*gui_register_menu_section)(GuiMenuSectionCallback callback, void* userdata);
    bool (*gui_show_notification)(const char* message);

    bool (*gui_ui_heading)(void* ui, const char* text);
    bool (*gui_ui_label)(void* ui, const char* text);
    bool (*gui_ui_small)(void* ui, const char* text);
    bool (*gui_ui_separator)(void* ui);
    bool (*gui_ui_button)(void* ui, const char* text);
    bool (*gui_ui_small_button)(void* ui, const char* text);
    bool (*gui_ui_checkbox)(void* ui, const char* text, bool* value);
    bool (*gui_ui_text_edit_singleline)(void* ui, char* buffer, size_t buffer_len);
    bool (*gui_ui_horizontal)(void* ui, GuiUiCallback callback, void* userdata);
    bool (*gui_ui_grid)(void* ui, const char* id, size_t columns, float sp_x, float sp_y, GuiUiCallback callback, void* userdata);
    bool (*gui_ui_end_row)(void* ui);
    bool (*gui_ui_colored_label)(void* ui, uint8_t r, uint8_t g, uint8_t b, uint8_t a, const char* text);

    bool (*gui_register_menu_item_icon)(const char* label, const char* icon_uri, const uint8_t* icon_ptr, size_t icon_len);
    bool (*gui_register_menu_section_with_icon)(const char* title, const char* icon_uri, const uint8_t* icon_ptr, size_t icon_len, GuiMenuSectionCallback callback, void* userdata);

    uint64_t (*android_dex_load)(const uint8_t* dex_ptr, size_t dex_len, const char* class_name);
    bool (*android_dex_unload)(uint64_t handle);
    bool (*android_dex_call_static_noargs)(uint64_t handle, const char* method, const char* sig);
    bool (*android_dex_call_static_string)(uint64_t handle, const char* method, const char* sig, const char* arg);
};

struct HachimiVtableV3 : public HachimiVtable {
    bool (*gui_ui_searchable_combobox)(void* ui, const char* id_salt, int* selected_value, const int* item_values, const char** item_labels, size_t item_count);
};

extern const HachimiVtable* g_hachimi;
extern int32_t g_hachimi_version;
