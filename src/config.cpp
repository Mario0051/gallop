#include "gallop.hpp"
#include <filesystem>
#include <fstream>
#include <toml.hpp>
#include <toml11/serializer.hpp>

namespace gallop {
gallop_config_t default_config = {{}};
gallop_config_t conf;

int init_config()
{
	const std::string path = "hachimi\\gallop_config.toml";
	toml::value toml_file;

	if (!std::filesystem::exists(path)) {
		spdlog::info("[config] No config found, generating gallop_config.toml");

		if (!std::filesystem::exists("hachimi\\"))
			std::filesystem::create_directory("hachimi\\");

		toml_file = default_config;
		std::ofstream f;
		f.open(path, std::ofstream::out | std::ofstream::trunc);
		f << toml::format(toml_file);
		f.close();
	} else {
		spdlog::info("[config] Config found (gallop_config.toml)");
		toml_file = toml::parse(path);
	}

	conf = toml::get<gallop_config_s>(toml_file);
	spdlog::info("[config] checking replaceCharacters");
	for (const auto& i : conf.replaceCharacters) {
		spdlog::info("[config] replaceCharacters[{}]", i.first);
		spdlog::info("[config] charaId: {}", i.second.charaId);
	}

	return 0;
}
} // namespace gallop
