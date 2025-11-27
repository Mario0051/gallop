#pragma once

#include "sqlite_modern_cpp.h"

namespace gallop {
// database handling (master.mdb and meta)
extern sqlite::database master;
extern sqlite::database meta;
int init_mdb();
void deinit_mdb();

} // namespace gallop
