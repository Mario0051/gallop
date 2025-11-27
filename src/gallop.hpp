#pragma once

#include "imgui_sink.hpp"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "sqlite3mc.h"
#include <sqlite_modern_cpp.h>
#include <stdbool.h>
#include <string>
#include <toml.hpp>
#include <unordered_map>

#include "config.hpp"
#include "gui.hpp"
#include "hook.hpp"
#include "mdb.hpp"

// Defines the gallop namespace.

namespace gallop {
// runs when gallop is attached
void attach();
// runs when gallop is detached
void detach();

extern std::filesystem::path path;

// sink and logger
extern std::shared_ptr<spdlog::logger> logger;
extern std::shared_ptr<gui::imgui_sink_mt> sink;
} // namespace gallop
