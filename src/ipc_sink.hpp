#pragma once
#include "spdlog/sinks/base_sink.h"
#include <windows.h>
#include <mutex>

namespace gallop {
class IpcSink : public spdlog::sinks::base_sink<std::mutex> {
    HANDLE hPipe;
public:
    IpcSink() {
        hPipe = CreateFile(L"\\\\.\\pipe\\GallopLogPipe", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    }

    ~IpcSink() {
        if (hPipe != INVALID_HANDLE_VALUE) CloseHandle(hPipe);
    }

    void sink_it_(const spdlog::details::log_msg& msg) override {
        if (hPipe == INVALID_HANDLE_VALUE) {
            hPipe = CreateFile(L"\\\\.\\pipe\\GallopLogPipe", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (hPipe == INVALID_HANDLE_VALUE) return;
        }

        spdlog::memory_buf_t formatted;
        base_sink<std::mutex>::formatter_->format(msg, formatted);

        DWORD written;
        WriteFile(hPipe, formatted.data(), formatted.size(), &written, NULL);
    }

    void flush_() override {}
};
}