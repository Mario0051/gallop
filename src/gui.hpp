#pragma once

namespace gallop {
// For GUI handling
namespace gui {
int init();
int update();
int destroy();
int run(); // Runs init+update+destroy together in one loop
} // namespace gui
} // namespace gallop
