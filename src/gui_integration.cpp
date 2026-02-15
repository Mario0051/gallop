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

static char new_orig_id[64] = "";
static char new_target_chara[64] = "";
static char new_target_dress[64] = "";
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
    char orig_chara[64] = "";
    char target_chara[64] = "";
    char target_dress[64] = "";
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
        show_overrides_section = !show_overrides_section;
    }

    if (show_overrides_section) {
        g_hachimi->gui_ui_small(ui, "Original Character ID:");
        g_hachimi->gui_ui_text_edit_singleline(ui, new_orig_id, sizeof(new_orig_id));
        int orig_id = parse_id(new_orig_id);
        if (orig_id != 0) {
            std::string name = get_chara_name_simple(orig_id);
            if (!name.empty()) g_hachimi->gui_ui_small(ui, name.c_str());
        }

        g_hachimi->gui_ui_small(ui, "Target Character ID:");
        g_hachimi->gui_ui_text_edit_singleline(ui, new_target_chara, sizeof(new_target_chara));
        int target_chara_id = parse_id(new_target_chara);
        if (target_chara_id != 0) {
            std::string name = get_chara_name_simple(target_chara_id);
            if (!name.empty()) g_hachimi->gui_ui_small(ui, name.c_str());
        }

        g_hachimi->gui_ui_small(ui, "Target Dress ID:");
        g_hachimi->gui_ui_text_edit_singleline(ui, new_target_dress, sizeof(new_target_dress));
        int target_dress_id = parse_id(new_target_dress);
        if (target_dress_id != 0) {
            std::string name = get_dress_name_simple(target_dress_id);
            if (!name.empty()) g_hachimi->gui_ui_small(ui, name.c_str());
        }

        g_hachimi->gui_ui_horizontal(ui, add_new_checkboxes_gui, nullptr);

        if (g_hachimi->gui_ui_button(ui, "Add Override")) {
            if (orig_id != 0) {
                gallop::gallop_char_info_t info;
                info.charaId = target_chara_id;
                info.clothId = target_dress_id;
                info.replaceMini = new_replace_mini;
                info.homeScreenOnly = new_home_only;

                std::string key = std::to_string(orig_id);
                gallop::conf.replaceCharacters[key] = info;

                new_orig_id[0] = '\0';
                new_target_chara[0] = '\0';
                new_target_dress[0] = '\0';
                new_replace_mini = false;
                new_home_only = false;
            }
        }

        g_hachimi->gui_ui_separator(ui);

        std::vector<std::string> to_remove;
        std::vector<std::pair<std::string, std::string>> to_rename;

        for (auto& [key, info] : gallop::conf.replaceCharacters) {
            EditState& state = edit_states[key];

            if (!state.initialized) {
                snprintf(state.orig_chara, sizeof(state.orig_chara), "%s", key.c_str());

                if (info.charaId != 0) snprintf(state.target_chara, sizeof(state.target_chara), "%d", info.charaId);
                else state.target_chara[0] = '\0';

                if (info.clothId != 0) snprintf(state.target_dress, sizeof(state.target_dress), "%d", info.clothId);
                else state.target_dress[0] = '\0';

                state.initialized = true;
            }

            RowContext ctx = { &key, &info, &state, false, false };

            g_hachimi->gui_ui_horizontal(ui, item_header_gui, &ctx);

            if (state.expanded) {
                g_hachimi->gui_ui_small(ui, "Base Character ID to Replace:");

                auto edit_base_id_gui = [](void* inner_ui, void* userdata) {
                    auto* c = static_cast<RowContext*>(userdata);
                    g_hachimi->gui_ui_text_edit_singleline(inner_ui, c->state->orig_chara, sizeof(c->state->orig_chara));
                    if (g_hachimi->gui_ui_button(inner_ui, "Apply Base ID")) {
                        c->apply_rename_clicked = true;
                    }
                };
                g_hachimi->gui_ui_horizontal(ui, edit_base_id_gui, &ctx);

                g_hachimi->gui_ui_small(ui, "Target Character ID:");
                g_hachimi->gui_ui_text_edit_singleline(ui, state.target_chara, sizeof(state.target_chara));
                info.charaId = parse_id(state.target_chara); 

                g_hachimi->gui_ui_small(ui, "Target Dress ID (0 for Default):");
                g_hachimi->gui_ui_text_edit_singleline(ui, state.target_dress, sizeof(state.target_dress));
                info.clothId = parse_id(state.target_dress); 
            } else {
                std::string info_str;
                if (info.clothId != 0) {
                    std::string dname = get_dress_name_simple(info.clothId);
                    info_str = fmt::format("Dress: {} ({})", dname, info.clothId);
                } else {
                    info_str = "Dress: Default/Auto";
                }
                g_hachimi->gui_ui_small(ui, info_str.c_str());
            }

            g_hachimi->gui_ui_horizontal(ui, item_flags_gui, &ctx);

            g_hachimi->gui_ui_separator(ui);

            if (ctx.remove_clicked) {
                to_remove.push_back(key);
            }
            if (ctx.apply_rename_clicked) {
                std::string new_key(state.orig_chara);
                if (!new_key.empty() && new_key != key) {
                    to_rename.push_back({key, new_key});
                }
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
