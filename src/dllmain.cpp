#include <filesystem>
#include <thread>
#include <windows.h>
#include <fstream>

#include "discord.hpp"
#include "gallop.hpp"
#include "ipc_sink.hpp"
#include "gui_binary.h"

#include "MinHook.h"

namespace gallop {
std::shared_ptr<spdlog::logger> logger;
std::filesystem::path path;

void ExtractAndRunGUI() {
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::filesystem::path exePath = std::filesystem::path(tempPath) / "gallop_gui_embed.exe";

    std::ofstream outfile(exePath, std::ios::binary);
    outfile.write((const char*)gallop_gui_embed_exe, gallop_gui_embed_exe_len);
    outfile.close();

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    if (CreateProcessW(exePath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        MessageBoxW(NULL, L"Failed to launch GUI process", L"Gallop Error", MB_OK | MB_ICONERROR);
    }
}

void attach()
{
	ExtractAndRunGUI();

	for (int i = 0; i < 200; i++) {
		if (WaitNamedPipeW(L"\\\\.\\pipe\\GallopLogPipe", 1)) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// Initialize spdlog
	auto ipc_sink = std::make_shared<gallop::IpcSink>();
	logger = std::make_shared<spdlog::logger>("base_logger", ipc_sink);

	spdlog::set_default_logger(logger);
	spdlog::set_pattern("[%l] %v");

	spdlog::info("[gallop] Successfully attached!");

	// Initialize config
	init_config();
	if (MH_Initialize() != MH_OK) {
		spdlog::error("[gallop] Failed to initialize minhook!");
		return;
	}
	il2cpp::init();
	discord::initialize();
	MH_EnableHook(MH_ALL_HOOKS);
	init_mdb();
}
void detach()
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
	discord::deinitialize();
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
