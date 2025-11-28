#pragma once

#include "sqlite_modern_cpp.h"
#include <map>

namespace gallop {
// Model data
extern std::unordered_map<int, int> dress2head;
extern std::unordered_map<int, int> dress2mini;
extern std::unordered_map<int, std::vector<int>> chara2dress;
extern std::map<int, std::string> id2name;
extern std::map<int, std::string> id2dress;

// database handling (master.mdb and meta)
extern sqlite::database master;
extern sqlite::database meta;
int init_mdb();
void deinit_mdb();

} // namespace gallop
