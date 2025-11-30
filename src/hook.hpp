#pragma once

#include <aetherim/field.hpp>
#include <aetherim/wrapper.hpp>
#include <memory>
#include <string>

// Various defines to define new hooks.
// clang-format off
#define GALLOP_HOOK_DEF(functionname, returntype, signature) \
	extern void* functionname##_orig; \
	returntype functionname##_hook signature;
#define GALLOP_CALL_ORIG(functionname) \
	reinterpret_cast<decltype(functionname##_hook)*>(functionname##_orig)
#define GALLOP_SETUP_HOOK_FOR_FUNC(functionname, returntype) \
	void* functionname##_orig; \
	returntype functionname##_hook
// clang-format on

namespace gallop {
// IL2CPP handling
namespace il2cpp {
// Wrapper for some il2cpp functions
extern std::unique_ptr<Wrapper> wrapper;
// umamusume.dll image
extern Image* umaimg;

// Inits the il2cpp functions
int init();

// Creates a new hook from a specified namespace, class and method, returning the original function.
void* create_hook(std::string namespaze, std::string class_name, std::string method, int method_args, void* destination);

// Returns an instance variable from a class.
void* get_class_from_instance(const void* instance);

// Reads a field from a class, field taken from an instance.
template <typename T = void*>
	requires std::is_trivial_v<T>
T read_field(const void* ptr, const Field* field)
{
	T result;
	const auto fieldPtr = static_cast<const std::byte*>(ptr) + field->get_offset();
	std::memcpy(std::addressof(result), fieldPtr, sizeof(T));
	return result;
}

// Writes a field to a class, field taken from an instance.
template <typename T>
	requires std::is_trivial_v<T>
void write_field(void* ptr, const Field* field, const T& value)
{
	const auto fieldPtr = static_cast<std::byte*>(ptr) + field->get_offset();
	std::memcpy(fieldPtr, std::addressof(value), sizeof(T));
}

// hooks
namespace hooks {
// Model hooks //

// StoryCharacter3D.LoadModel
GALLOP_HOOK_DEF(StoryCharacter3D_LoadModel, void,
				(int charaId, int cardId, int clothId, int zekkenNumber, int headId, bool isWet, bool isDirt, int mobId, int dressColorId,
				 int charaDressColorSetId, void* zekkenName, int zekkenFontStyle, int color, int fontColor, int suitColor, bool isUseDressDataHeadModelSubId,
				 bool useCircleShadow, int wetTexturePartsFlag))
// SingleModeSceneController.CreateModel
GALLOP_HOOK_DEF(SingleModeSceneController_CreateModel, void*, (void* _this, int cardId, int dressId, bool addVoiceCue))
// CharacterBuildInfo() (1)
GALLOP_HOOK_DEF(CharacterBuildInfo_ctor_0, void,
				(void* _this, int charaId, int dressId, int controllerType, int headId, int zekken, int mobId, int backDancerColorId,
				 bool isUseDressDataHeadModelSubId, int audienceId, int motionDressId, bool isEnableModelCache))
// CharacterBuildInfo() (2)
GALLOP_HOOK_DEF(CharacterBuildInfo_ctor_1, void,
				(void* _this, int cardId, int charaId, int dressId, int controllerType, int headId, int zekken, int mobId, int backDancerColorId,
				 int overrideClothCategory, bool isUseDressDataHeadModelSubId, int audienceId, int motionDressId, bool isEnableModelCache,
				 int charaDressColorSetId))
// CharacterBuildInfo.Rebuild
GALLOP_HOOK_DEF(CharacterBuildInfo_Rebuild, void, (void* _this))
// WorkSingleModeCharaData.GetRaceDressId
GALLOP_HOOK_DEF(WorkSingleModeCharaData_GetRaceDressId, int, (void* _this, bool isApplyDressChange))
// EditableCharacterBuildInfo()
GALLOP_HOOK_DEF(EditableCharacterBuildInfo_ctor, void,
				(void* _this, int cardId, int charaId, int dressId, int controllerType, int zekken, int mobId, int backDancerColorId, int headId,
				 bool isUseDressDataHeadModelSubId, bool isEnableModelCache, int chara_dress_color_set_id))

void init_model_hooks();

// Menu related hooks //

// HomeViewController.ChangeHeader
GALLOP_HOOK_DEF(HomeViewController_ChangeHeader, void, (void* _this, int state, int displayTypeId, void* onComplete))

void init_menu_hooks();
} // namespace hooks
} // namespace il2cpp
} // namespace gallop
