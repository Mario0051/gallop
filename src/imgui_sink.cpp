#include "imgui_sink.hpp"
#include "imgui.h"

namespace gallop {
namespace gui {
// TODO color support
void ImGuiSink::sink_it_(const spdlog::details::log_msg& msg)
{
	spdlog::memory_buf_t formatted;
	spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);
	std::string str = fmt::to_string(formatted);
	log.append(str.c_str());
	_scroll = true;
}

void ImGuiSink::flush_() {}

void ImGuiSink::Draw()
{
	const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	if (ImGui::BeginChild("ScrollRegion##", ImVec2(0, -footerHeightToReserve), false, 0)) {
		ImGui::PushTextWrapPos();
		ImGui::TextUnformatted(log.c_str());
		if (_scroll) {
			ImGui::SetScrollHereY(1.0f);
			_scroll = false;
		}
		ImGui::EndChild();
	}
}
} // namespace gui
} // namespace gallop
