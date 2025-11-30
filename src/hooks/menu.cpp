#include "discord.hpp"
#include "gallop.hpp"
#include "hook.hpp"

enum class HomeTopState {
	None = 0,
	MyPage = 1,
	Character = 2,
	Story = 3,
	Race = 4,
	Gacha = 5,
	LoginBonus = 7,
	TalkMode = 8,
	Poster = 9,
	Jukebox = 10,
	Valentine = 11,
	MAX = 12,
};

namespace gallop {
namespace il2cpp {
namespace hooks {
GALLOP_SETUP_HOOK_FOR_FUNC(HomeViewController_ChangeHeader, void)(void* _this, int state, int displayTypeId, void* onComplete)
{
	spdlog::info("[hooks/menu] Changed home state! {}", state);
	HomeTopState newState = static_cast<HomeTopState>(state);
	switch (newState) {
	case HomeTopState::MyPage:
	case HomeTopState::LoginBonus:
	case HomeTopState::TalkMode:
	case HomeTopState::Poster:
	case HomeTopState::Valentine:
		gallop::discord::setRichPresence("Home", "Main Menu");
		break;
	case HomeTopState::Character:
		gallop::discord::setRichPresence("Upgrade", "Main Menu");
		break;
	case HomeTopState::Story:
		gallop::discord::setRichPresence("Story", "Main Menu");
		break;
	case HomeTopState::Race:
		gallop::discord::setRichPresence("Race", "Main Menu");
		break;
	case HomeTopState::Gacha:
		gallop::discord::setRichPresence("Gacha", "Main Menu");
		break;
	default:
		break;
	}
	return GALLOP_CALL_ORIG(HomeViewController_ChangeHeader)(_this, state, displayTypeId, onComplete);
}

void init_menu_hooks()
{
	HomeViewController_ChangeHeader_orig =
		gallop::il2cpp::create_hook("Gallop", "HomeViewController", "ChangeHeader", 3, reinterpret_cast<void*>(HomeViewController_ChangeHeader_hook));
}
} // namespace hooks
} // namespace il2cpp
} // namespace gallop
