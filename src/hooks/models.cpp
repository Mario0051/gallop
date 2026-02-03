#include "gallop.hpp"
#include "hook.hpp"
#include "hachimi_api.h"
#include "config.hpp"
#include "mdb.hpp"
#include <string>
#include <string>
#include <spdlog/spdlog.h>

using namespace gallop;
using namespace gallop::il2cpp;

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
	if (dressID < 100000) replaceDress = false;
	std::string strId = std::to_string(charaID);
	if (gallop::conf.replaceCharacters.contains(strId)) {
		spdlog::info("[hooks/models] Attempting to replace model for character ID {} (dress ID {}, controller Type {})", charaID, dressID, (int)controllerType);
		gallop::gallop_char_info_t charInfo = gallop::conf.replaceCharacters.at(strId);
		if (charInfo.charaId == 0) return false;
		if (charInfo.homeScreenOnly && (controllerType < UmaControllerType::HomeStand || controllerType > UmaControllerType::HomeWalk))
			return false;
		if (controllerType == UmaControllerType::Mini && charInfo.replaceMini) {
			if (gallop::dress2mini.contains(dressID)) {
				charaID = charInfo.charaId;
				if (replaceDress) dressID = charInfo.clothId;
				if (gallop::dress2head.contains(dressID)) headID = gallop::dress2head.at(dressID);
			}
			spdlog::info("[hooks/models] Successfully replaced mini character! (charaID: {}, dressID: {})", charaID, dressID);
			return true;
		} else if (controllerType == UmaControllerType::Mini && !charInfo.replaceMini)
			return false;
		charaID = charInfo.charaId;
		// Manually specified
		if (charInfo.clothId)
			dressID = charInfo.clothId;
		else if (dressID >= 1000) // dressID not specified, so pick out dressID ourselves
		{
			spdlog::info("[hooks/models] Determining new dressID for character ID {}", charaID);
			if (gallop::chara2dress.contains(charaID)) {
				// Handle casual outfits separately
				if (dressID >= 900000) {
					// Default to school outfit if none was found!
					dressID = 5;
					for (auto& dress : gallop::chara2dress.at(charaID)) {
						if (dress >= 900000) {
							dressID = dress;
							break;
						}
					}
				} else if (dressID >= 100000) {
					// All other outfits use the format <chara_id><race_outfit_id>
					// For now assume that all character IDs are 4 characters long
					std::string strDressID = std::to_string(dressID).substr(4);
					// Default to school outfit if none was found!
					dressID = 5;
					bool has_seen_default = false;
					for (auto& dress : gallop::chara2dress.at(charaID)) {
						int dressID2 = std::stoi(std::to_string(dress).substr(4));
						// Found first outfit
						if (dressID2 == 1) has_seen_default = true;
						if (dressID2 == std::stoi(strDressID)) {
							dressID = dress;
							break;
						}
					}
					// If we are referring to an alt outfit and we dont have a matching one, default to the base outfit if we have it
					if (has_seen_default && dressID == 5) {
						dressID = (charaID * 100) + 1;
					}
				}
			}
		}
		if (gallop::dress2head.contains(dressID))
			headID = gallop::dress2head.at(dressID);
		return true;
	}
	return false;
}

bool ReplaceCharacterController(int& cardID, int& charaID, int& dressID, int& headID, UmaControllerType controllerType)
{
	if (ReplaceCharacterController(charaID, dressID, headID, controllerType)) {
		if (cardID >= 1000) {
			if ((cardID / 100) != charaID) {
				cardID = charaID * 100 + 1;
			}
		}
		return true;
	}
	return false;
}

namespace gallop {
namespace il2cpp {
namespace hooks {

GALLOP_SETUP_HOOK_FOR_FUNC(StoryCharacter3D_LoadModel, void)(int charaId, int cardId, int clothId, int zekkenNumber, int headId, bool isWet, bool isDirt,
															 int mobId, int dressColorId, int charaDressColorSetId, void* zekkenName, int zekkenFontStyle,
															 int color, int fontColor, int suitColor, bool isUseDressDataHeadModelSubId, bool useCircleShadow,
															 int wetTexturePartsFlag)
{
	ReplaceCharacterController(cardId, charaId, clothId, headId, UmaControllerType::ORIG);
	return GALLOP_CALL_ORIG(StoryCharacter3D_LoadModel)(charaId, cardId, clothId, zekkenNumber, headId, isWet, isDirt, mobId, dressColorId,
														charaDressColorSetId, zekkenName, zekkenFontStyle, color, fontColor, suitColor,
														isUseDressDataHeadModelSubId, useCircleShadow, wetTexturePartsFlag);
}
GALLOP_SETUP_HOOK_FOR_FUNC(CharacterBuildInfo_ctor_0, void)(void* _this, int charaId, int dressId, int controllerType, int headId, int zekken, int mobId,
															int backDancerColorId, bool isUseDressDataHeadModelSubId, int audienceId, int motionDressId,
															bool isEnableModelCache)
{
	ReplaceCharacterController(charaId, dressId, headId, static_cast<UmaControllerType>(controllerType));
	return GALLOP_CALL_ORIG(CharacterBuildInfo_ctor_0)(_this, charaId, dressId, controllerType, headId, zekken, mobId, backDancerColorId,
													   isUseDressDataHeadModelSubId, audienceId, motionDressId, isEnableModelCache);
}
GALLOP_SETUP_HOOK_FOR_FUNC(CharacterBuildInfo_ctor_1, void)(void* _this, int cardId, int charaId, int dressId, int controllerType, int headId, int zekken,
															int mobId, int backDancerColorId, int overrideClothCategory, bool isUseDressDataHeadModelSubId,
															int audienceId, int motionDressId, bool isEnableModelCache, int charaDressColorSetId)
{
	ReplaceCharacterController(cardId, charaId, dressId, headId, static_cast<UmaControllerType>(controllerType));
	return GALLOP_CALL_ORIG(CharacterBuildInfo_ctor_1)(_this, cardId, charaId, dressId, controllerType, headId, zekken, mobId, backDancerColorId,
													   overrideClothCategory, isUseDressDataHeadModelSubId, audienceId, motionDressId, isEnableModelCache,
													   charaDressColorSetId);
}

#define SET_FIELD_AND_READ(_this, this_class, var, type) \
	static FieldInfo* var##_field = nullptr; \
	if (!var##_field) { \
		var##_field = g_hachimi->il2cpp_get_field_from_name((Il2CppClass*)this_class, ("_" + std::string(#var)).c_str()); \
	} \
	auto var = gallop::il2cpp::read_field<type>(_this, var##_field);

GALLOP_SETUP_HOOK_FOR_FUNC(CharacterBuildInfo_Rebuild, void)(void* _this)
{
	void* this_class = gallop::il2cpp::get_class_from_instance(_this);
	if (!this_class) return GALLOP_CALL_ORIG(CharacterBuildInfo_Rebuild)(_this);

	// Define all the values from this class
	SET_FIELD_AND_READ(_this, this_class, cardId, int)
	SET_FIELD_AND_READ(_this, this_class, charaId, int)
	SET_FIELD_AND_READ(_this, this_class, dressId, int)
	SET_FIELD_AND_READ(_this, this_class, controllerType, int)
	SET_FIELD_AND_READ(_this, this_class, headModelSubId, int)
	SET_FIELD_AND_READ(_this, this_class, motionDressId, int)

	// Call ReplaceCharacterController and write all fields
	if (ReplaceCharacterController(charaId, dressId, headModelSubId, static_cast<UmaControllerType>(controllerType))) {
		gallop::il2cpp::write_field(_this, charaId_field, charaId);
		gallop::il2cpp::write_field(_this, dressId_field, dressId);
		gallop::il2cpp::write_field(_this, headModelSubId_field, headModelSubId);
		gallop::il2cpp::write_field(_this, motionDressId_field, dressId);
		gallop::il2cpp::write_field(_this, cardId_field, -1);
	}

	return GALLOP_CALL_ORIG(CharacterBuildInfo_Rebuild)(_this);
}
GALLOP_SETUP_HOOK_FOR_FUNC(WorkSingleModeCharaData_GetRaceDressId, int)(void* _this, bool isApplyDressChange)
{
	int ret = GALLOP_CALL_ORIG(WorkSingleModeCharaData_GetRaceDressId)(_this, false);
	if ((ret > 100000) && (ret <= 999999)) {
		int charaId;
		if (ret / 10000 == 90) {
			charaId = ret % 10000;
		} else {
			charaId = ret / 100;
		}
		int newDressId = ret;
		int newHeadId = 0;
		if (ReplaceCharacterController(charaId, newDressId, newHeadId, UmaControllerType::ORIG)) {
			return newDressId;
		}
	}
	return ret;
}
GALLOP_SETUP_HOOK_FOR_FUNC(EditableCharacterBuildInfo_ctor, void)(void* _this, int cardId, int charaId, int dressId, int controllerType, int zekken, int mobId,
																  int backDancerColorId, int headId, bool isUseDressDataHeadModelSubId, bool isEnableModelCache,
																  int chara_dress_color_set_id)
{
	ReplaceCharacterController(cardId, charaId, dressId, headId, static_cast<UmaControllerType>(controllerType));
	return GALLOP_CALL_ORIG(EditableCharacterBuildInfo_ctor)(_this, cardId, charaId, dressId, controllerType, zekken, mobId, backDancerColorId, headId,
															 isUseDressDataHeadModelSubId, isEnableModelCache, chara_dress_color_set_id);
}
void init_model_hooks()
{
	// StoryCharacter3D.LoadModel
	StoryCharacter3D_LoadModel_orig =
		gallop::il2cpp::create_hook("Gallop", "StoryCharacter3D", "LoadModel", 18, reinterpret_cast<void*>(StoryCharacter3D_LoadModel_hook));
	// CharacterBuildInfo..ctor(s)
	CharacterBuildInfo_ctor_0_orig =
		gallop::il2cpp::create_hook("Gallop", "CharacterBuildInfo", ".ctor", 11, reinterpret_cast<void*>(CharacterBuildInfo_ctor_0_hook));
	CharacterBuildInfo_ctor_1_orig =
		gallop::il2cpp::create_hook("Gallop", "CharacterBuildInfo", ".ctor", 14, reinterpret_cast<void*>(CharacterBuildInfo_ctor_1_hook));
	// CharacterBuildInfo.Rebuild
	CharacterBuildInfo_Rebuild_orig =
		gallop::il2cpp::create_hook("Gallop", "CharacterBuildInfo", "Rebuild", 0, reinterpret_cast<void*>(CharacterBuildInfo_Rebuild_hook));
	// WorkSingleModeCharaData.GetRaceDressId
	WorkSingleModeCharaData_GetRaceDressId_orig = gallop::il2cpp::create_hook("Gallop", "WorkSingleModeCharaData", "GetRaceDressId", 1,
																			  reinterpret_cast<void*>(WorkSingleModeCharaData_GetRaceDressId_hook));
	// EditableCharacterBuildInfo.ctor
	EditableCharacterBuildInfo_ctor_orig =
		gallop::il2cpp::create_hook("Gallop", "EditableCharacterBuildInfo", ".ctor", 11, reinterpret_cast<void*>(EditableCharacterBuildInfo_ctor_hook));
}
} // namespace hooks
} // namespace il2cpp
} // namespace gallop
