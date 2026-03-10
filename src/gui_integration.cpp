#include "hachimi_api.h"
#include "config.hpp"
#include "discord.hpp"
#include "mdb.hpp"
#include "gallop.hpp"

#include <fmt/format.h>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>

static int parse_id(const char* buffer) {
    try {
        if (buffer && strlen(buffer) > 0) {
            return std::stoi(buffer);
        }
    } catch (...) {}
    return 0;
}

static std::string get_chara_name_simple(int id) {
    if (id == 0) return "";
    if (gallop::id2name.contains(id)) {
        return gallop::id2name.at(id);
    }
    return "Unknown Character";
}

static std::string get_dress_name_simple(int id) {
    if (id == 0) return "";
    if (id == 5) return "Tracem Uniform (Winter)";
    if (gallop::id2dress.contains(id)) {
        return gallop::id2dress.at(id);
    }
    return "Unknown Dress";
}

static int new_orig_id = 0;
static char new_orig_id_str[64] = "";
static int new_target_chara = 0;
static char new_target_chara_str[64] = "";
static int new_target_dress = 0;
static char new_target_dress_str[64] = "";
static bool new_replace_mini = false;
static bool new_home_only = false;

static bool show_overrides_section = false;
static bool show_about_section = false;

static void add_new_checkboxes_gui(void* ui, void* userdata) {
    g_hachimi->gui_ui_checkbox(ui, "Replace Mini", &new_replace_mini);
    g_hachimi->gui_ui_checkbox(ui, "Home Only", &new_home_only);
}

struct EditState {
    bool expanded = false;
    int new_base_id = 0;
    char orig_chara_str[64] = "";
    char target_chara_str[64] = "";
    char target_dress_str[64] = "";
    bool initialized = false;
};

static std::unordered_map<std::string, EditState> edit_states;

struct RowContext {
    const std::string* key;
    gallop::gallop_char_info_t* info;
    EditState* state;
    bool remove_clicked;
    bool apply_rename_clicked;
};

static std::string fmt_chara_display(int id) {
    if (gallop::id2name.contains(id)) {
        return fmt::format("{} ({})", gallop::id2name.at(id), id);
    }
    return std::to_string(id);
}

static void item_header_gui(void* ui, void* userdata) {
    auto* ctx = static_cast<RowContext*>(userdata);
    int orig_id = parse_id(ctx->key->c_str());

    std::string label = fmt::format("{} -> {}", fmt_chara_display(orig_id), fmt_chara_display(ctx->info->charaId));
    g_hachimi->gui_ui_label(ui, label.c_str());

    std::string edit_label = ctx->state->expanded ? fmt::format("▼ Edit##{}", *ctx->key) : fmt::format("▶ Edit##{}", *ctx->key);
    if (g_hachimi->gui_ui_button(ui, edit_label.c_str())) {
        ctx->state->expanded = !ctx->state->expanded;
    }

    std::string btn_label = fmt::format("Remove##{}", *ctx->key);
    if (g_hachimi->gui_ui_button(ui, btn_label.c_str())) {
        ctx->remove_clicked = true;
    }
}

static void item_flags_gui(void* ui, void* userdata) {
    auto* ctx = static_cast<RowContext*>(userdata);
    std::string mini_label = fmt::format("Replace Mini##{}", *ctx->key);
    std::string home_label = fmt::format("Home Only##{}", *ctx->key);

    g_hachimi->gui_ui_checkbox(ui, mini_label.c_str(), &ctx->info->replaceMini);
    g_hachimi->gui_ui_checkbox(ui, home_label.c_str(), &ctx->info->homeScreenOnly);
}

static void config_buttons_gui(void* ui, void* userdata) {
    if (g_hachimi->gui_ui_button(ui, "Load Config")) {
        if (gallop::init_config() == 0) {
            g_hachimi->gui_show_notification("Gallop: Config loaded.");
        } else {
            g_hachimi->gui_show_notification("Gallop: Failed to load config!");
        }
    }
    if (g_hachimi->gui_ui_button(ui, "Save Config")) {
        if (gallop::save_config() == 0) {
            g_hachimi->gui_show_notification("Gallop: Config saved.");
        } else {
            g_hachimi->gui_show_notification("Gallop: Failed to save config!");
        }
    }
}

void render_gallop_settings(void* ui, void* userdata) {
    static std::vector<int> char_ids;
    static std::vector<const char*> char_labels;
    static std::vector<std::string> char_labels_str;

    static std::vector<int> dress_ids;
    static std::vector<const char*> dress_labels;
    static std::vector<std::string> dress_labels_str;

    if (char_ids.empty() && !gallop::id2name.empty()) {
        char_ids.reserve(gallop::id2name.size() + 1);
        char_labels_str.reserve(gallop::id2name.size() + 1);
        char_labels.reserve(gallop::id2name.size() + 1);

        char_ids.push_back(0);
        char_labels_str.push_back("None (0)");

        for (const auto& [id, name] : gallop::id2name) {
            if (id == 0) continue;
            char_ids.push_back(id);
            char_labels_str.push_back(fmt::format("{} ({})", name, id));
        }

        for (const auto& str : char_labels_str) {
            char_labels.push_back(str.c_str());
        }
    }

    if (dress_ids.empty() && !gallop::id2dress.empty()) {
        dress_ids.reserve(gallop::id2dress.size() + 1);
        dress_labels_str.reserve(gallop::id2dress.size() + 1);
        dress_labels.reserve(gallop::id2dress.size() + 1);

        dress_ids.push_back(0);
        dress_labels_str.push_back("Default/Auto (0)");

        for (const auto& [id, name] : gallop::id2dress) {
            if (id == 0) continue;
            dress_ids.push_back(id);
            dress_labels_str.push_back(fmt::format("{} ({})", name, id));
        }

        for (const auto& str : dress_labels_str) {
            dress_labels.push_back(str.c_str());
        }
    }

    g_hachimi->gui_ui_heading(ui, "Gallop Settings");

    if (g_hachimi->gui_ui_checkbox(ui, "Enable Discord RPC", &gallop::conf.discordRPC)) {
        if (gallop::conf.discordRPC) {
            gallop::discord::initialize();
        } else {
            gallop::discord::deinitialize();
        }
    }

    g_hachimi->gui_ui_separator(ui);

    const char* overrides_label = show_overrides_section ? "▼ Character Overrides" : "▶ Character Overrides";
    if (g_hachimi->gui_ui_button(ui, overrides_label)) {
        if (!show_overrides_section && g_hachimi_version >= 3) {
            auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
            if (v3->gui_save_menu_width) v3->gui_save_menu_width();
        }
        show_overrides_section = !show_overrides_section;
        if (!show_overrides_section && g_hachimi_version >= 3) {
            auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
            if (v3->gui_restore_menu_width) v3->gui_restore_menu_width();
        }
    }

    if (show_overrides_section) {
        g_hachimi->gui_ui_small(ui, "Original Character ID:");
        if (g_hachimi_version >= 3) {
            auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
            if (v3->gui_ui_searchable_combobox) {
                v3->gui_ui_searchable_combobox(ui, "new_orig_chara", &new_orig_id, char_ids.data(), char_labels.data(), char_ids.size());
            }
        } else {
            g_hachimi->gui_ui_text_edit_singleline(ui, new_orig_id_str, sizeof(new_orig_id_str));
            new_orig_id = parse_id(new_orig_id_str);
            if (new_orig_id != 0) {
                std::string name = get_chara_name_simple(new_orig_id);
                if (!name.empty()) g_hachimi->gui_ui_small(ui, name.c_str());
            }
        }

        g_hachimi->gui_ui_small(ui, "Target Character ID:");
        if (g_hachimi_version >= 3) {
            auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
            if (v3->gui_ui_searchable_combobox) {
                v3->gui_ui_searchable_combobox(ui, "new_target_chara", &new_target_chara, char_ids.data(), char_labels.data(), char_ids.size());
            }
        } else {
            g_hachimi->gui_ui_text_edit_singleline(ui, new_target_chara_str, sizeof(new_target_chara_str));
            new_target_chara = parse_id(new_target_chara_str);
            if (new_target_chara != 0) {
                std::string name = get_chara_name_simple(new_target_chara);
                if (!name.empty()) g_hachimi->gui_ui_small(ui, name.c_str());
            }
        }

        g_hachimi->gui_ui_small(ui, "Target Dress ID:");
        if (g_hachimi_version >= 3) {
            auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
            if (v3->gui_ui_searchable_combobox) {
                v3->gui_ui_searchable_combobox(ui, "new_target_dress", &new_target_dress, dress_ids.data(), dress_labels.data(), dress_ids.size());
            }
        } else {
            g_hachimi->gui_ui_text_edit_singleline(ui, new_target_dress_str, sizeof(new_target_dress_str));
            new_target_dress = parse_id(new_target_dress_str);
            if (new_target_dress != 0) {
                std::string name = get_dress_name_simple(new_target_dress);
                if (!name.empty()) g_hachimi->gui_ui_small(ui, name.c_str());
            }
        }

        g_hachimi->gui_ui_horizontal(ui, add_new_checkboxes_gui, nullptr);

        if (g_hachimi->gui_ui_button(ui, "Add Override")) {
            if (new_orig_id != 0) {
                gallop::gallop_char_info_t info;
                info.charaId = new_target_chara;
                info.clothId = new_target_dress;
                info.replaceMini = new_replace_mini;
                info.homeScreenOnly = new_home_only;

                std::string key = std::to_string(new_orig_id);
                gallop::conf.replaceCharacters[key] = info;

                new_orig_id = 0;
                new_target_chara = 0;
                new_target_dress = 0;
                new_orig_id_str[0] = '\0';
                new_target_chara_str[0] = '\0';
                new_target_dress_str[0] = '\0';
                new_replace_mini = false;
                new_home_only = false;
            }
        }
        g_hachimi->gui_ui_separator(ui);

        std::vector<std::string> to_remove;
        std::vector<std::pair<std::string, std::string>> to_rename;

        for (auto& [key, info] : gallop::conf.replaceCharacters) {
            EditState& state = edit_states[key];
            int orig_id = parse_id(key.c_str());

            if (!state.initialized) {
                state.new_base_id = orig_id;
                snprintf(state.orig_chara_str, sizeof(state.orig_chara_str), "%s", key.c_str());
                if (info.charaId != 0) snprintf(state.target_chara_str, sizeof(state.target_chara_str), "%d", info.charaId);
                if (info.clothId != 0) snprintf(state.target_dress_str, sizeof(state.target_dress_str), "%d", info.clothId);
                state.initialized = true;
            }

            RowContext ctx = { &key, &info, &state, false, false };

            g_hachimi->gui_ui_horizontal(ui, [](void* inner_ui, void* userdata) {
                auto* c = static_cast<RowContext*>(userdata);
                int orig_id = parse_id(c->key->c_str());

                std::string label = fmt::format("{} -> {}", fmt_chara_display(orig_id), fmt_chara_display(c->info->charaId));
                g_hachimi->gui_ui_label(inner_ui, label.c_str());
            }, &ctx);

            g_hachimi->gui_ui_horizontal(ui, [](void* inner_ui, void* userdata) {
                auto* c = static_cast<RowContext*>(userdata);
                std::string edit_label = c->state->expanded ? fmt::format("▼ Edit##{}", *c->key) : fmt::format("▶ Edit##{}", *c->key);
                if (g_hachimi->gui_ui_button(inner_ui, edit_label.c_str())) {
                    if (!c->state->expanded && g_hachimi_version >= 3) {
                        auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
                        if (v3->gui_save_menu_width) v3->gui_save_menu_width();
                    }
                    c->state->expanded = !c->state->expanded;
                    if (!c->state->expanded && g_hachimi_version >= 3) {
                        auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
                        if (v3->gui_restore_menu_width) v3->gui_restore_menu_width();
                    }
                }
                std::string btn_label = fmt::format("Remove##{}", *c->key);
                if (g_hachimi->gui_ui_button(inner_ui, btn_label.c_str())) c->remove_clicked = true;
            }, &ctx);

            if (state.expanded) {
                g_hachimi->gui_ui_small(ui, "Base Character ID to Replace:");
                g_hachimi->gui_ui_horizontal(ui, [](void* inner_ui, void* userdata) {
                    auto* c = static_cast<RowContext*>(userdata);
                    if (g_hachimi_version >= 3) {
                        auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
                        if (v3->gui_ui_searchable_combobox) {
                            v3->gui_ui_searchable_combobox(inner_ui, (*c->key + "_base").c_str(), &c->state->new_base_id, char_ids.data(), char_labels.data(), char_ids.size());
                        }
                    } else {
                        g_hachimi->gui_ui_text_edit_singleline(inner_ui, c->state->orig_chara_str, sizeof(c->state->orig_chara_str));
                        c->state->new_base_id = parse_id(c->state->orig_chara_str);
                    }
                    if (g_hachimi->gui_ui_button(inner_ui, "Apply Base ID")) c->apply_rename_clicked = true;
                }, &ctx);
                if (g_hachimi_version < 3 && state.new_base_id != 0) {
                    std::string name = get_chara_name_simple(state.new_base_id);
                    if (!name.empty()) g_hachimi->gui_ui_small(ui, name.c_str());
                }
                g_hachimi->gui_ui_separator(ui);

                g_hachimi->gui_ui_small(ui, "Target Character:");
                if (g_hachimi_version >= 3) {
                    auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
                    if (v3->gui_ui_searchable_combobox) {
                        v3->gui_ui_searchable_combobox(ui, (key + "_chara").c_str(), &info.charaId, char_ids.data(), char_labels.data(), char_ids.size());
                    }
                } else {
                    g_hachimi->gui_ui_text_edit_singleline(ui, state.target_chara_str, sizeof(state.target_chara_str));
                    info.charaId = parse_id(state.target_chara_str);
                    if (info.charaId != 0) {
                        std::string name = get_chara_name_simple(info.charaId);
                        if (!name.empty()) g_hachimi->gui_ui_small(ui, name.c_str());
                    }
                }

                g_hachimi->gui_ui_small(ui, "Target Dress:");
                if (g_hachimi_version >= 3) {
                    auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
                    if (v3->gui_ui_searchable_combobox) {
                        v3->gui_ui_searchable_combobox(ui, (key + "_dress").c_str(), &info.clothId, dress_ids.data(), dress_labels.data(), dress_ids.size());
                    }
                } else {
                    g_hachimi->gui_ui_text_edit_singleline(ui, state.target_dress_str, sizeof(state.target_dress_str));
                    info.clothId = parse_id(state.target_dress_str);
                    if (info.clothId != 0) {
                        std::string name = get_dress_name_simple(info.clothId);
                        if (!name.empty()) g_hachimi->gui_ui_small(ui, name.c_str());
                    }
                }

                std::string mini_label = fmt::format("Replace Mini##{}", key);
                std::string home_label = fmt::format("Home Only##{}", key);
                g_hachimi->gui_ui_checkbox(ui, mini_label.c_str(), &info.replaceMini);
                g_hachimi->gui_ui_checkbox(ui, home_label.c_str(), &info.homeScreenOnly);
            } else {
                std::string info_str = info.clothId != 0 ? fmt::format("Dress: {} ({})", get_dress_name_simple(info.clothId), info.clothId) : "Dress: Default/Auto";
                g_hachimi->gui_ui_small(ui, info_str.c_str());
            }

            g_hachimi->gui_ui_separator(ui);

            if (ctx.remove_clicked) {
                to_remove.push_back(key);
                if (ctx.state->expanded && g_hachimi_version >= 3) {
                    auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
                    if (v3->gui_restore_menu_width) v3->gui_restore_menu_width();
                }
            }
            if (ctx.apply_rename_clicked) {
                std::string new_key = std::to_string(state.new_base_id);
                if (state.new_base_id != 0 && new_key != key) to_rename.push_back({key, new_key});
            }
        }

        for (const auto& key : to_remove) {
            gallop::conf.replaceCharacters.erase(key);
            edit_states.erase(key);
        }

        for (const auto& [old_k, new_k] : to_rename) {
            if (gallop::conf.replaceCharacters.contains(new_k)) continue;

            auto node = gallop::conf.replaceCharacters.extract(old_k);
            node.key() = new_k;
            gallop::conf.replaceCharacters.insert(std::move(node));

            edit_states[new_k] = edit_states[old_k];
            edit_states.erase(old_k);
        }
    }

    g_hachimi->gui_ui_horizontal(ui, config_buttons_gui, nullptr);

    g_hachimi->gui_ui_separator(ui);

    const char* about_label = show_about_section ? "▼ About" : "▶ About";
    if (g_hachimi->gui_ui_button(ui, about_label)) {
        show_about_section = !show_about_section;
    }

    if (show_about_section) {
        g_hachimi->gui_ui_label(ui, "Trainers' Gallop U");
        g_hachimi->gui_ui_small(ui, "Version 0.1-NOTFORPROD");
        g_hachimi->gui_ui_small(ui, "Created by Yonii Hayasaka (haya).");

        g_hachimi->gui_ui_separator(ui);

        g_hachimi->gui_ui_label(ui, "Libraries used:");
        g_hachimi->gui_ui_small(ui, "- JSON for Modern C++ (nlohmann/json)");
        g_hachimi->gui_ui_small(ui, "- spdlog (gabime/spdlog)");
        g_hachimi->gui_ui_small(ui, "- toml11 (ToruNiina/toml11)");
        g_hachimi->gui_ui_small(ui, "- Discord RPC (harmonytf/discord-rpc)");
        g_hachimi->gui_ui_small(ui, "- {fmt} (fmtlib/fmt)");
        g_hachimi->gui_ui_small(ui, "- SQLite3 Multiple Ciphers");

        g_hachimi->gui_ui_separator(ui);

        g_hachimi->gui_ui_label(ui, "Special thanks to:");
        g_hachimi->gui_ui_small(ui, "- jack, Mario9581, Hachimi Discord");
    }
}

namespace gallop {
    void init_gui_integration() {
        g_hachimi->gui_register_menu_section(render_gallop_settings, nullptr);
    }
}
