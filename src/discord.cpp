#include "discord.hpp"
#include "config.hpp"
#include <ctime>
#include <discord_rpc.h>
#include <spdlog/spdlog.h>

namespace gallop {
namespace discord {

static void HandleDiscordReady(const DiscordUser* connectedUser) { spdlog::info("[rpc] Connected to {} ({})", connectedUser->username, connectedUser->userId); }
static void HandleDiscordDisconnected(int errcode, const char* message) { spdlog::info("[rpc] Disconnected code {} ({})", errcode, message); }
static void HandleDiscordError(int errcode, const char* message) { spdlog::info("[rpc] Error code {} ({})", errcode, message); }
static void HandleDiscordJoin(const char* secret) {}
static void HandleDiscordSpectate(const char* secret) {}
static void HandleDiscordJoinRequest(const DiscordUser* request) {}

void initialize()
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));

	handlers.ready = HandleDiscordReady;
	handlers.disconnected = HandleDiscordDisconnected;
	handlers.errored = HandleDiscordError;
	handlers.joinGame = HandleDiscordJoin;
	handlers.spectateGame = HandleDiscordSpectate;
	handlers.joinRequest = HandleDiscordJoinRequest;

	spdlog::info("[rpc] Setting up Discord RPC...");
	Discord_Initialize(CLIENT_ID, &handlers, 1, NULL);

	// set rich presence at startup
	setRichPresence("Title Screen", "Main Menu", "umaicon", "It's Special Week!", time(0));
}

void deinitialize()
{
	Discord_ClearPresence();
	Discord_Shutdown();
}

void setRichPresence(std::string state, std::string details, std::string largeImageKey, std::string largeImageText, time_t start)
{
	if (!gallop::conf.discordRPC)
		return;

	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));

	discordPresence.details = details.c_str();
	discordPresence.state = state.c_str();
	discordPresence.largeImageKey = largeImageKey.c_str();
	discordPresence.largeImageText = largeImageText.c_str();
	if (start >= 0)
		discordPresence.startTimestamp = start;
	Discord_UpdatePresence(&discordPresence);
}
} // namespace discord
} // namespace gallop
