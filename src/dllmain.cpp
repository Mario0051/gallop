#include <filesystem>
#include <thread>
#include <windows.h>

#include "gallop.hpp"
#include "imgui_sink.hpp"

#include "MinHook.h"

namespace gallop {
std::shared_ptr<spdlog::logger> logger;
std::shared_ptr<gui::imgui_sink_mt> sink;
std::filesystem::path path;

void attach()
{
	// Initialize spdlog
	sink = std::make_shared<gui::imgui_sink_mt>();
	logger = std::make_shared<spdlog::logger>("base_logger", sink);

	spdlog::set_default_logger(logger);
	spdlog::set_pattern("[%l] %v");

	spdlog::info("[gallop] Successfully attached!");

	std::thread(gui::run).detach();

	// Initialize config
	init_config();
	if (MH_Initialize() != MH_OK) {
		spdlog::error("[gallop] Failed to initialize minhook!");
		return;
	}
	il2cpp::init();
	MH_EnableHook(MH_ALL_HOOKS);
	init_mdb();
}
void detach()
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
	deinit_mdb();
}
} // namespace gallop

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	WCHAR buffer[MAX_PATH];
	const std::filesystem::path module_path(std::wstring(buffer, GetModuleFileName(nullptr, buffer, MAX_PATH)));
	if (module_path.filename() == L"umamusume.exe" || module_path.filename() == L"UmamusumePrettyDerby_Jpn.exe") {
		current_path(module_path.parent_path());
		gallop::path = module_path.parent_path();

		if (ul_reason_for_call == DLL_PROCESS_ATTACH)
			std::thread(gallop::attach).detach();
		if (ul_reason_for_call == DLL_PROCESS_DETACH)
			gallop::detach();
	}
	return TRUE;
}
