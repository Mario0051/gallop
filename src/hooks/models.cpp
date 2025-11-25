#include "gallop.hpp"
#include <safetyhook.hpp>

enum class UmaControllerType {
	Default = 0x0,
	Race = 0x1,
	Training = 0x2,
	EventTimeline = 0x3,
	Live = 0x4,
	LiveTheater = 0x5,
	HomeStand = 0x6,
	HomeTalk = 0x7,
	HomeWalk = 0x8,
	CutIn = 0x9,
	TrainingTop = 0xa,
	SingleRace = 0xb,
	Simple = 0xc,
	Mini = 0xd,
	Paddock = 0xe,
	Champions = 0xf,
	ORIG = 0x1919810
};

bool ReplaceCharacterController(int& charaID, int& dressID, int& headID, UmaControllerType controllerType)
{
	bool replaceDress = true;
	if (dressID < 100000)
		replaceDress = false;
	std::string strId = std::to_string(charaID);
	spdlog::info("[hooks/models] Attempting to replace model for character ID {} (dress ID {})", charaID, dressID);
	if (gallop::conf.replaceCharacters.contains(strId)) {
		gallop::gallop_char_info_t charInfo = gallop::conf.replaceCharacters.at(std::to_string(charaID));
		if (charaID == 9001 && charInfo.homeScreenOnly && controllerType == UmaControllerType::HomeStand)
			return false; // Can't change Tazuna
		if (controllerType == UmaControllerType::Mini && charInfo.replaceMini) {
			if (gallop::dress2mini.contains(dressID) && gallop::dress2mini.contains(dressID)) {
				charaID = charInfo.charaId;
				if (replaceDress)
					dressID = charInfo.clothId;
				if (gallop::dress2head.contains(dressID))
					headID = gallop::dress2head.at(dressID);
			}
			spdlog::info("[hooks/models] Successfully replaced mini character! (charaID: {}, dressID: {})", charaID, dressID);
			return true;
		} else if (controllerType == UmaControllerType::Mini && !charInfo.replaceMini)
			return false;
		charaID = charInfo.charaId;
		if (charInfo.clothId)
			dressID = charInfo.clothId;
		if (gallop::dress2head.contains(dressID))
			headID = gallop::dress2head.at(dressID);
		spdlog::info("[hooks/models] Successfully replaced character! (charaID: {}, dressID: {})", charaID, dressID);
	}
	return true;
}

bool ReplaceCharacterController(int& cardID, int& charaID, int& dressID, int& headID, UmaControllerType controllerType)
{
	if (ReplaceCharacterController(charaID, dressID, headID, controllerType)) {
		if (cardID >= 1000) {
			if ((cardID / 100) != charaID) {
				cardID = charaID * 100 + 1;
			}
		}
		spdlog::info("[hooks/models] New cardID! (cardID: {}, charaID: {})", cardID, charaID);
		return true;
	}
	return false;
}

namespace gallop {
namespace il2cpp {
namespace hooks {
SETUP_HOOK_FOR_FUNC(StoryCharacter3D_LoadModel, void)(int charaId, int cardId, int clothId, int zekkenNumber, int headId, bool isWet, bool isDirt, int mobId,
													  int dressColorId, int charaDressColorSetId, void* zekkenName, int zekkenFontStyle, int color,
													  int fontColor, int suitColor, bool isUseDressDataHeadModelSubId, bool useCircleShadow,
													  int wetTexturePartsFlag)
{
	ReplaceCharacterController(cardId, charaId, clothId, headId, UmaControllerType::ORIG);
	return CALL_ORIG(StoryCharacter3D_LoadModel)(charaId, cardId, clothId, zekkenNumber, headId, isWet, isDirt, mobId, dressColorId, charaDressColorSetId,
												 zekkenName, zekkenFontStyle, color, fontColor, suitColor, isUseDressDataHeadModelSubId, useCircleShadow,
												 wetTexturePartsFlag);
}
void init_model_hooks()
{
	// StoryCharacter3D.LoadModel
	StoryCharacter3D_LoadModel_orig =
		gallop::il2cpp::create_hook("Gallop", "StoryCharacter3D", "LoadModel", 18, reinterpret_cast<void*>(StoryCharacter3D_LoadModel_hook));
}
} // namespace hooks
} // namespace il2cpp
} // namespace gallop
