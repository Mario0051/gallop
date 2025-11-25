#pragma once

#include "imgui_sink.hpp"
#include "nlohmann/json.hpp"
#include "safetyhook.hpp"
#include "spdlog/spdlog.h"
#include "sqlite3mc.h"
#include <sqlite_modern_cpp.h>
#include <stdbool.h>
#include <string>
#include <unordered_map>

// clang-format off
#define HOOK_DEF(functionname, returntype, signature) \
	extern void* functionname##_orig; \
	returntype functionname##_hook signature;
#define CALL_ORIG(functionname) \
	reinterpret_cast<decltype(functionname##_hook)*>(functionname##_orig)
#define SETUP_HOOK_FOR_FUNC(functionname, returntype) \
	void* functionname##_orig; \
	returntype functionname##_hook
// clang-format on

// Defines the gallop namespace.

namespace gallop {
// runs when gallop is attached
void attach();
// runs when gallop is detached
void detach();

extern std::filesystem::path path;

// sink and logger
extern std::shared_ptr<spdlog::logger> logger;
extern std::shared_ptr<gui::imgui_sink_mt> sink;

// config
int init_config();

typedef struct gallop_char_info_s {
	int charaId = 0;
	int clothId = 0;
	bool replaceMini = false;
	bool homeScreenOnly = false;
} gallop_char_info_t;
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(gallop_char_info_s, charaId, clothId, replaceMini, homeScreenOnly)

typedef struct gallop_config_s {
	std::unordered_map<std::string, gallop_char_info_t> replaceCharacters;
} gallop_config_t;
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(gallop_config_s, replaceCharacters)

extern gallop_config_t conf;

// For GUI handling
namespace gui {
int init();
int update();
int destroy();
int run(); // Runs init+update+destroy together in one loop
} // namespace gui

// database handling (master.mdb and meta)
extern sqlite::database master;
extern sqlite::database meta;
int init_mdb();
void deinit_mdb();

// Model data
extern std::unordered_map<int, int> dress2head;
extern std::unordered_map<int, int> dress2mini;

// IL2CPP handling
namespace il2cpp {
int init();
void* create_hook(std::string namespaze, std::string class_name, std::string method, int method_args, void* destination);
// hooks
namespace hooks {
// Model hooks //

// StoryCharacter3D.LoadModel
HOOK_DEF(StoryCharacter3D_LoadModel, void,
		 (int charaId, int cardId, int clothId, int zekkenNumber, int headId, bool isWet, bool isDirt, int mobId, int dressColorId, int charaDressColorSetId,
		  void* zekkenName, int zekkenFontStyle, int color, int fontColor, int suitColor, bool isUseDressDataHeadModelSubId, bool useCircleShadow,
		  int wetTexturePartsFlag))
// SingleModeSceneController.CreateModel
HOOK_DEF(SingleModeSceneController_CreateModel, void*, (void* _this, int cardId, int dressId, bool addVoiceCue))
// CharacterBuildInfo() (1)
HOOK_DEF(CharacterBuildInfo_ctor_0, void,
		 (void* _this, int charaId, int dressId, int controllerType, int headId, int zekken, int mobId, int backDancerColorId,
		  bool isUseDressDataHeadModelSubId, int audienceId, int motionDressId, bool isEnableModelCache))
// CharacterBuildInfo() (2)
HOOK_DEF(CharacterBuildInfo_ctor_1, void,
		 (void* _this, int cardId, int charaId, int dressId, int controllerType, int headId, int zekken, int mobId, int backDancerColorId,
		  int overrideClothCategory, bool isUseDressDataHeadModelSubId, int audienceId, int motionDressId, bool isEnableModelCache, int charaDressColorSetId))
// CharacterBuildInfo.Rebuild
HOOK_DEF(CharacterBuildInfo_Rebuild, void, (void* _this))
// WorkSingleModeCharaData.GetRaceDressId
HOOK_DEF(WorkSingleModeCharaData_GetRaceDressId, int, (void* _this, bool isApplyDressChange))
// EditableCharacterBuildInfo()
HOOK_DEF(EditableCharacterBuildInfo_ctor, void,
		 (void* _this, int cardId, int charaId, int dressId, int controllerType, int zekken, int mobId, int backDancerColorId, int headId,
		  bool isUseDressDataHeadModelSubId, bool isEnableModelCache, int chara_dress_color_set_id))

void init_model_hooks();
} // namespace hooks
} // namespace il2cpp
} // namespace gallop
