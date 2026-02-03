#include <filesystem>
#include <mutex>
#include <thread>
#include <windows.h>

#include "config.hpp"
#include "discord.hpp"
#include "gallop.hpp"
#include "hachimi_api.h"
#include "mdb.hpp"
#include "hook.hpp"

#include "spdlog/sinks/base_sink.h"

const HachimiVtable* g_hachimi = nullptr;

namespace gallop {
std::shared_ptr<spdlog::logger> logger;
std::filesystem::path path;

template <typename Mutex>
class hachimi_sink : public spdlog::sinks::base_sink<Mutex> {
  protected:
	void sink_it_(const spdlog::details::log_msg& msg) override
	{
		if (!g_hachimi)
			return;

		spdlog::memory_buf_t formatted;
		spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

		int32_t level = 3;
		if (msg.level == spdlog::level::err)
			level = 1;
		else if (msg.level == spdlog::level::warn)
			level = 2;
		else if (msg.level == spdlog::level::debug)
			level = 4;
		else if (msg.level == spdlog::level::trace)
			level = 5;

		g_hachimi->log(level, "gallop", fmt::to_string(formatted).c_str());
	}

	void flush_() override {}
};

void attach()
{
	spdlog::info("[gallop] Attaching...");

	// Initialize config
	init_config();

	if (il2cpp::init() != 0) {
		spdlog::error("[gallop] Failed to initialize il2cpp!");
		return;
	}

	if (conf.discordRPC)
		discord::initialize();

	init_mdb();

	init_gui_integration();

	spdlog::info("[gallop] Initialization complete!");
}

void detach()
{
	if (conf.discordRPC)
		discord::deinitialize();
	deinit_mdb();
}
} // namespace gallop

extern "C" __declspec(dllexport) InitResult hachimi_init(const HachimiVtable* vtable, int version)
{
	g_hachimi = vtable;

	// Initialize spdlog
	auto hachimi_logger_sink = std::make_shared<gallop::hachimi_sink<std::mutex>>();
	gallop::logger = std::make_shared<spdlog::logger>("base_logger", hachimi_logger_sink);
	spdlog::set_default_logger(gallop::logger);
	spdlog::set_pattern("[%l] %v");

	gallop::path = std::filesystem::current_path();

	spdlog::info("[gallop] Hachimi init (Version: {})", version);

	gallop::attach();

	return InitResult::Ok;
}
