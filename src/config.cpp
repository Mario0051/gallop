#include "config.hpp"
#include "gallop.hpp"
#include <fstream>
#include <spdlog/spdlog.h>
#include <filesystem>

namespace gallop {

gallop_config_t default_config = {true, {}};
gallop_config_t conf;

int init_config()
{
	std::filesystem::path config_dir = gallop::path / "hachimi";
	std::filesystem::path config_path = config_dir / "gallop_config.toml";

	if (!std::filesystem::exists(config_path)) {
		spdlog::info("[config] No config found, generating hachimi/gallop_config.toml");

		if (!std::filesystem::exists(config_dir)) {
			std::error_code ec;
			std::filesystem::create_directory(config_dir, ec);
			if (ec) {
				 spdlog::error("[config] Failed to create directory: {}", ec.message());
				 return 1;
			}
		}

		conf = default_config;
		save_config();
		return 0;
	}

	try {
		spdlog::info("[config] Loading config from {}", config_path.string());
		auto toml_file = toml::parse(config_path.string());
		conf = toml::get<gallop_config_s>(toml_file);
	} catch (std::exception& e) {
		spdlog::error("[gallop] Config error: {}", e.what());
		conf = default_config;
		return 1;
	}
	return 0;
}

int save_config()
{
	std::filesystem::path config_dir = gallop::path / "hachimi";
	std::filesystem::path config_path = config_dir / "gallop_config.toml";

	if (!std::filesystem::exists(config_dir)) {
		 std::filesystem::create_directory(config_dir);
	}

	toml::value toml_data = conf;
	std::ofstream file(config_path);
	if (!file.is_open()) return 1;
	file << toml_data;
	return 0;
}

} // namespace gallop
