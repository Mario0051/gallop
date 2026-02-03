#pragma once

#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "sqlite3mc.h"
#include <sqlite_modern_cpp.h>
#include <stdbool.h>
#include <string>
#include <toml.hpp>
#include <unordered_map>
#include <filesystem>

#include "hachimi_api.h"

// Defines the gallop namespace.

namespace gallop {
// runs when gallop is attached
void attach();
// runs when gallop is detached
void detach();

void init_gui_integration();

extern std::filesystem::path path;

// logger
extern std::shared_ptr<spdlog::logger> logger;
} // namespace gallop

extern const HachimiVtable* g_hachimi;
