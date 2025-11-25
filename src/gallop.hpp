#pragma once

#include "imgui_sink.hpp"
#include "spdlog/spdlog.h"
#include <stdbool.h>
#include <string>

// Defines the gallop namespace.

namespace gallop {
// runs when gallop is attached
void attach();
// runs when gallop is detached
void detach();

// sink and logger
extern std::shared_ptr<spdlog::logger> logger;
extern std::shared_ptr<gui::imgui_sink_mt> sink;

// For GUI handling
namespace gui {
int init();
int update();
int destroy();
int run(); // Runs init+update+destroy together in one loop
} // namespace gui

// IL2CPP handling
namespace il2cpp {
int init();
}
} // namespace gallop
