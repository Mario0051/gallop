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
#include "spdlog/async.h"

const HachimiVtable* g_hachimi = nullptr;
int32_t g_hachimi_version = 0;

static HachimiVtableV3 g_dynamic_vtable = {};

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

InitResult internal_init(int version) {
	// Initialize spdlog
	spdlog::init_thread_pool(8192, 1);
	auto hachimi_logger_sink = std::make_shared<gallop::hachimi_sink<std::mutex>>();
	gallop::logger = std::make_shared<spdlog::async_logger>(
		"base_logger",
		hachimi_logger_sink,
		spdlog::thread_pool(),
		spdlog::async_overflow_policy::overrun_oldest
	);
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

extern "C" HACHIMI_EXPORT InitResult hachimi_init(const HachimiVtable* vtable, int version)
{
	g_hachimi = vtable;
	g_hachimi_version = version;

	return internal_init(version);
}

extern "C" HACHIMI_EXPORT InitResult hachimi_init_v3(HachimiGetApiFn get_api, int version)
{
	g_hachimi_version = version;

	g_dynamic_vtable.log = (decltype(g_dynamic_vtable.log))get_api("log");
	g_dynamic_vtable.hachimi_instance = (decltype(g_dynamic_vtable.hachimi_instance))get_api("hachimi_instance");
	g_dynamic_vtable.hachimi_get_interceptor = (decltype(g_dynamic_vtable.hachimi_get_interceptor))get_api("hachimi_get_interceptor");
	g_dynamic_vtable.interceptor_hook = (decltype(g_dynamic_vtable.interceptor_hook))get_api("interceptor_hook");

	g_dynamic_vtable.il2cpp_get_assembly_image = (decltype(g_dynamic_vtable.il2cpp_get_assembly_image))get_api("il2cpp_get_assembly_image");
	g_dynamic_vtable.il2cpp_get_class = (decltype(g_dynamic_vtable.il2cpp_get_class))get_api("il2cpp_get_class");
	g_dynamic_vtable.il2cpp_get_method_addr = (decltype(g_dynamic_vtable.il2cpp_get_method_addr))get_api("il2cpp_get_method_addr");
	g_dynamic_vtable.il2cpp_get_field_from_name = (decltype(g_dynamic_vtable.il2cpp_get_field_from_name))get_api("il2cpp_get_field_from_name");

	g_dynamic_vtable.il2cpp_get_field_value = (decltype(g_dynamic_vtable.il2cpp_get_field_value))get_api("il2cpp_get_field_value");
	g_dynamic_vtable.il2cpp_set_field_value = (decltype(g_dynamic_vtable.il2cpp_set_field_value))get_api("il2cpp_set_field_value");

	g_dynamic_vtable.gui_register_menu_section = (decltype(g_dynamic_vtable.gui_register_menu_section))get_api("gui_register_menu_section");
	g_dynamic_vtable.gui_show_notification = (decltype(g_dynamic_vtable.gui_show_notification))get_api("gui_show_notification");
	g_dynamic_vtable.gui_ui_heading = (decltype(g_dynamic_vtable.gui_ui_heading))get_api("gui_ui_heading");
	g_dynamic_vtable.gui_ui_label = (decltype(g_dynamic_vtable.gui_ui_label))get_api("gui_ui_label");
	g_dynamic_vtable.gui_ui_small = (decltype(g_dynamic_vtable.gui_ui_small))get_api("gui_ui_small");
	g_dynamic_vtable.gui_ui_separator = (decltype(g_dynamic_vtable.gui_ui_separator))get_api("gui_ui_separator");
	g_dynamic_vtable.gui_ui_button = (decltype(g_dynamic_vtable.gui_ui_button))get_api("gui_ui_button");
	g_dynamic_vtable.gui_ui_checkbox = (decltype(g_dynamic_vtable.gui_ui_checkbox))get_api("gui_ui_checkbox");
	g_dynamic_vtable.gui_ui_text_edit_singleline = (decltype(g_dynamic_vtable.gui_ui_text_edit_singleline))get_api("gui_ui_text_edit_singleline");
	g_dynamic_vtable.gui_ui_horizontal = (decltype(g_dynamic_vtable.gui_ui_horizontal))get_api("gui_ui_horizontal");

	g_dynamic_vtable.gui_ui_searchable_combobox = (decltype(g_dynamic_vtable.gui_ui_searchable_combobox))get_api("gui_ui_searchable_combobox");
	g_dynamic_vtable.gui_get_menu_width = (decltype(g_dynamic_vtable.gui_get_menu_width))get_api("gui_get_menu_width");
	g_dynamic_vtable.gui_set_menu_width = (decltype(g_dynamic_vtable.gui_set_menu_width))get_api("gui_set_menu_width");
	g_dynamic_vtable.hachimi_get_base_dir = (decltype(g_dynamic_vtable.hachimi_get_base_dir))get_api("hachimi_get_base_dir");
	g_dynamic_vtable.hachimi_get_data_path = (decltype(g_dynamic_vtable.hachimi_get_data_path))get_api("hachimi_get_data_path");

	g_hachimi = &g_dynamic_vtable;

	return internal_init(version);
}
