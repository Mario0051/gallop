#include "gallop.hpp"
#include <filesystem>
#include <fstream>

namespace gallop {
gallop_config_t default_config = {{}};
gallop_config_t conf;

int init_config()
{
	const std::string path = "hachimi\\gallop_config.json";
	nlohmann::json config;

	if (!std::filesystem::exists(path)) {
		spdlog::info("[config] No config found, generating gallop_config.json");

		if (!std::filesystem::exists("hachimi\\"))
			std::filesystem::create_directory("hachimi\\");

		config = default_config;
		std::ofstream f;
		f.open(path, std::ofstream::out | std::ofstream::trunc);

		f << config.dump(4);

		f.close();
	} else {
		spdlog::info("[config] Config found (gallop_config.json)");

		std::ifstream f(path);
		config = nlohmann::json::parse(f);
	}

	spdlog::info("[config] config: {}", config.dump());

	conf = config.get<gallop_config_s>();
	spdlog::info("[config] checking replaceCharacters");
	for (const auto& i : conf.replaceCharacters) {
		spdlog::info("[config] replaceCharacters[{}]", i.first);
		spdlog::info("[config] charaId: {}", i.second.charaId);
	}

	return 0;
}
} // namespace gallop
