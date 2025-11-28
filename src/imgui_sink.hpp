#pragma once

#include "imgui.h"
#include "spdlog/sinks/base_sink.h"

#include <mutex>

namespace gallop {
namespace gui {
class ImGuiSink : public spdlog::sinks::base_sink<std::mutex> {
  public:
	void Draw();
	void Append(const char* text) {
		std::lock_guard<std::mutex> lock(mutex_);
		log.append(text);
	}

  protected:
	ImGuiTextBuffer log;

	void sink_it_(const spdlog::details::log_msg& msg) override;
	void flush_() override;
};

using imgui_sink_mt = ImGuiSink;
} // namespace gui
} // namespace gallop
