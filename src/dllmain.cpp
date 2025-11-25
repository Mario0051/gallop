#include <filesystem>
#include <thread>
#include <windows.h>

#include "gallop.hpp"
#include "imgui_sink.hpp"

namespace gallop {
std::shared_ptr<spdlog::logger> logger;
std::shared_ptr<gui::imgui_sink_mt> sink;

void attach()
{
	// Initialize spdlog
	sink = std::make_shared<gui::imgui_sink_mt>();
	logger = std::make_shared<spdlog::logger>("base_logger", sink);

	spdlog::set_default_logger(logger);
	spdlog::set_pattern("[%l] %v");

	spdlog::info("[gallop] Successfully attached!");

	il2cpp::init();

	std::thread(gui::run).detach();
}
void detach() {}
} // namespace gallop

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	WCHAR buffer[MAX_PATH];
	const std::filesystem::path module_path(std::wstring(buffer, GetModuleFileName(nullptr, buffer, MAX_PATH)));
	if (module_path.filename() == L"umamusume.exe" || module_path.filename() == L"UmamusumePrettyDerby_Jpn.exe") {
		if (ul_reason_for_call == DLL_PROCESS_ATTACH)
			std::thread(gallop::attach).detach();
		if (ul_reason_for_call == DLL_PROCESS_DETACH)
			gallop::detach();
	}
	return TRUE;
}
