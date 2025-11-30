#pragma once

#include <string>
#include <time.h>

namespace gallop {
namespace discord {
constexpr const char* CLIENT_ID = "954453106765225995";

void initialize();
void deinitialize();

void setRichPresence(std::string state, std::string details, std::string largeImageKey = "umaicon", std::string largeImageText = "It's Special Week!",
					 time_t start = -1);
// TODO set this up with race related hooks
void setRaceRichPresence();
} // namespace discord
} // namespace gallop
