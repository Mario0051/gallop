#include <windows.h>
#include <thread>
#include <vector>
#include <string>
#include <filesystem>
#include "imgui.h"
#include "imgui_sink.hpp"
#include "spdlog/spdlog.h"
#include "config.hpp"
#include "mdb.hpp"

namespace gallop {
    extern bool done;

    std::shared_ptr<gui::imgui_sink_mt> sink;

    std::filesystem::path path = std::filesystem::current_path();
}

namespace gallop::gui {
    int init();
    int update_and_paint();
    int destroy();
}

void PipeServerThread() {
    HANDLE hPipe = CreateNamedPipe(
        L"\\\\.\\pipe\\GallopLogPipe",
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1, 1024 * 16, 1024 * 16, 0, NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) return;

    if (ConnectNamedPipe(hPipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED) {
        char buffer[1024];
        DWORD bytesRead;
        while (!gallop::done) {
            if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                if (gallop::sink) {
                    gallop::sink->Append(buffer);
                }
            } else {
                DWORD err = GetLastError();
                if (err == ERROR_BROKEN_PIPE) {
                    gallop::done = true;
                    break;
                } else {
                    break;
                }
            }
        }
    }
    CloseHandle(hPipe);
}

extern "C" int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    gallop::sink = std::make_shared<gallop::gui::imgui_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("gui_logger", gallop::sink);
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%l] %v");

    std::thread pipeThread(PipeServerThread);
    pipeThread.detach();

    gallop::init_config();
    gallop::init_mdb();

    if (gallop::gui::init() != 0) return 1;

    while (!gallop::done) {
        gallop::gui::update_and_paint();
    }

    gallop::gui::destroy();
    return 0;
}