#include "gallop.hpp"
#include <codecvt>
#include <locale>
#include <map>
#include <nlohmann/json.hpp>
#include <sqlite3mc.h>
#include <sqlite_modern_cpp.h>
#include <unordered_map>
#include <utility>
#include <filesystem>
#include <vector>

#define DATABASE_KEY "9c2bab97bcf8c0c4f1a9ea7881a213f6c9ebf9d8d4c6a8e43ce5a259bde7e9fd"

namespace gallop {
// Maps dresses to head IDs
std::unordered_map<int, int> dress2head;
// Maps dresses to mini dress existence
std::unordered_map<int, int> dress2mini;
// Maps character IDs to dress IDs
std::unordered_map<int, std::vector<int>> chara2dress;

// TODO these could probably be vectors instead...
// Maps character IDs to character names
std::map<int, std::string> id2name;
// Maps dress IDs to dress names
std::map<int, std::string> id2dress;

int init_mdb()
{
	std::string pragma_prepare = ("PRAGMA hexkey='" + std::string(DATABASE_KEY) + "'");

	std::string master_path;
	std::string meta_path;
	std::filesystem::path data_path;

	if (g_hachimi_version >= 3) {
		auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);
		if (v3->hachimi_get_data_path) {
			const char* api_path = v3->hachimi_get_data_path();
			if (api_path != nullptr) {
				data_path = std::string(api_path);
			}
		}
	}

	if (data_path.empty()) {
#if defined(_WIN32) || defined(_WIN64)
		std::filesystem::path game_root = std::filesystem::current_path();

		std::vector<std::filesystem::path> possible_paths = {
			game_root / "UmamusumePrettyDerby_Jpn_Data" / "Persistent",
			game_root / "umamusume_Data" / "Persistent"
		};

		const char* user_profile = std::getenv("USERPROFILE");
        if (user_profile != nullptr) {
			std::filesystem::path local_low = std::filesystem::path(user_profile) / "AppData" / "LocalLow" / "Cygames";
			possible_paths.push_back(local_low / "umamusume");
			possible_paths.push_back(local_low / "UmamusumePrettyDerby_Jpn");
		}

		for (const auto& p : possible_paths) {
			std::error_code ec;
			if (std::filesystem::exists(p / "master" / "master.mdb", ec) && !ec) {
				data_path = p;
				break;
			}
		}

		if (data_path.empty()) data_path = game_root;
#else
		data_path = "/data/data/jp.co.cygames.umamusume/files";
#endif
	}

	master_path = (data_path / "master" / "master.mdb").string();
	meta_path = (data_path / "meta").string();

	sqlite::database master;
	sqlite::database meta;
	sqlite::sqlite_config config;
	config.flags = sqlite::OpenFlags::READONLY;

	// Open up master.mdb
	try {
		master = sqlite::database(master_path, config);
		spdlog::error("[mdb] master.mdb: {}", master_path);
	} catch (const std::exception& e) {
		spdlog::error("[mdb] master.mdb could not be opened! {}", e.what());
		return 1;
	}

	// open up meta
	try {
		meta = sqlite::database(meta_path, config);
		meta << pragma_prepare;
	} catch (const std::exception& e) {
		spdlog::error("[mdb] meta could not be opened! {}", e.what());
		return 1;
	}

	// Test if it works
	spdlog::info("[mdb] Testing master.mdb");
	try {
		std::string cipher;
		master << "SELECT id FROM dress_data";
	} catch (const std::exception& e) {
		spdlog::error("[mdb] master.mdb could not be tested! {}", e.what());
		return 1;
	}

	spdlog::info("[mdb] Testing meta");
	try {
		meta << "SELECT CAST(n AS TEXT) FROM a WHERE n LIKE '3d/chara/body/bdy0002_00/pfb_bdy%'";
	} catch (const std::exception& e) {
		spdlog::error("[mdb] meta could not be tested! {}", e.what());
	}

	spdlog::info("[mdb] Initialized successfully!");

	// Query dress -> head ids and dress -> mini ids
	try {
		master << "SELECT id, head_sub_id, have_mini FROM dress_data" >> [&](int id, int head_sub_id, int have_mini) {
			dress2mini.emplace(id, have_mini);
			dress2head.emplace(id, head_sub_id);
			// spdlog::info("[mdb] {} -> {}", id, head_sub_id);
		};
		master << "SELECT chara_id, id FROM dress_data" >> [&](int chara_id, int id) {
			if (!chara2dress.contains(chara_id)) {
				chara2dress.emplace(chara_id, std::vector<int>());
			}
			auto& dresses = chara2dress.at(chara_id);
			dresses.push_back(id);
		};
	} catch (const std::exception& e) {
		spdlog::error("[mdb] could not extract dress ids from master.mdb! {}", e.what());
	}

	// id2name has 0 (none)
	id2name.emplace(0, "None");
	// id2dress has 0 (no dress)
	id2dress.emplace(0, "Default");

	// Try querying character and dress names
	// Use localized_data over a result from a query
	if (std::filesystem::exists("hachimi\\localized_data") && std::filesystem::exists("hachimi\\localized_data\\text_data_dict.json")) {
		using namespace nlohmann;
		json j = json::parse(std::ifstream("hachimi\\localized_data\\text_data_dict.json"));
		for (const auto& oid : j.value("6", std::unordered_map<std::string, std::string>())) {
			const int id = std::stoi(oid.first);
			const std::string name = oid.second;
			id2name.emplace(id, name);
		}
		for (const auto& oid : j.value("14", std::unordered_map<std::string, std::string>())) {
			const int id = std::stoi(oid.first);
			const std::string name = oid.second;
			id2dress.emplace(id, name);
		}
	}
	// Now try a query for possibly missing names (untranslated)
	try {
		master << "SELECT id FROM chara_data" >> [&](int id) {
			if (!id2name.contains(id))
				master << "SELECT text FROM text_data WHERE `index` =? AND category=6" << id >> [&](std::string text) {
					id2name.emplace(id, text);
					spdlog::info("[mdb] Found untranslated character name! {}", text);
				};
		};
		master << "SELECT id FROM dress_data" >> [&](int id) {
			if (!id2dress.contains(id))
				master << "SELECT text FROM text_data WHERE `index`=? AND category=14" << id >> [&](std::string text) {
					id2dress.emplace(id, text);
					spdlog::info("[mdb] Found untranslated dress name! {}", text);
				};
		};
	} catch (const std::exception& e) {
		spdlog::error("[mdb] could not extract character names from master.mdb! {}", e.what());
	}
	// Alright haya now hit the second tower

	return 0;
}

void deinit_mdb() {}
} // namespace gallop
