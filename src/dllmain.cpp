#include <filesystem>
#include <mutex>
#include <thread>

#if defined(_WIN32) || defined(_WIN64)
    #define HACHIMI_EXPORT __declspec(dllexport)
#else
    #define HACHIMI_EXPORT __attribute__((visibility("default")))
#endif

#include "config.hpp"
#include "discord.hpp"
#include "gallop.hpp"
#include "hachimi_api.h"
#include "mdb.hpp"
#include "hook.hpp"

#include "spdlog/sinks/base_sink.h"
#include "spdlog/pattern_formatter.h"

const HachimiVtable* g_hachimi = nullptr;
int32_t g_hachimi_version = 0;

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

		formatted.push_back('\0'); 
		g_hachimi->log(level, "gallop", formatted.data());
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

extern "C" HACHIMI_EXPORT InitResult hachimi_init(const HachimiVtable* vtable, int version)
{
	g_hachimi = vtable;
	g_hachimi_version = version;

	// Initialize spdlog
	auto hachimi_logger_sink = std::make_shared<gallop::hachimi_sink<std::mutex>>();
	gallop::logger = std::make_shared<spdlog::logger>("base_logger", hachimi_logger_sink);
	spdlog::set_default_logger(gallop::logger);
	auto formatter = std::make_unique<spdlog::pattern_formatter>("[%l] %v", spdlog::pattern_time_type::local, "");
	spdlog::set_formatter(std::move(formatter));

	if (g_hachimi_version >= 3) {
        auto v3 = reinterpret_cast<const HachimiVtableV3*>(g_hachimi);

        if (v3->hachimi_get_base_dir) {
            const char* base_dir = v3->hachimi_get_base_dir();
            if (base_dir != nullptr) {
                gallop::path = std::string(base_dir);
            }
        }
    } else {
#if defined(_WIN32) || defined(_WIN64)
		gallop::path = std::filesystem::current_path() / "hachimi";
#else
		const char* possible_paths[] = {
			"/storage/emulated/0/Android/media/jp.co.cygames.umamusume/hachimi",
			"/sdcard/Android/media/jp.co.cygames.umamusume/hachimi",
			"/data/local/tmp/hachimi"
		};

		gallop::path = possible_paths[0];
		for (const char* p : possible_paths) {
			std::error_code ec;
			if (std::filesystem::exists(p, ec) && !ec) {
				gallop::path = p;
				break;
			}
		}
#endif
	}

	spdlog::info("[gallop] Hachimi init (Version: {})", version);

	gallop::attach();

	return InitResult::Ok;
}
