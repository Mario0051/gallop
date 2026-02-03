#include "hook.hpp"
#include "gallop.hpp"
#include "hachimi_api.h"
#include <spdlog/spdlog.h>
#include <string>

namespace gallop {
namespace il2cpp {

int init() {
	spdlog::info("[gallop] Initializing hooks...");

	hooks::init_model_hooks();
	hooks::init_menu_hooks();
	return 0;
}

// Creates a hook that returns a reference to the original function
void* create_hook(std::string namespaze, std::string class_name, std::string method, int method_args, void* destination) {
	if (!g_hachimi) return nullptr;

	const Il2CppImage* image = g_hachimi->il2cpp_get_assembly_image("umamusume.dll");
	if (!image) image = g_hachimi->il2cpp_get_assembly_image("Assembly-CSharp.dll");
	if (!image) return nullptr;

	Il2CppClass* klass = g_hachimi->il2cpp_get_class(image, namespaze.c_str(), class_name.c_str());
	if (!klass) return nullptr;

	void* target_addr = g_hachimi->il2cpp_get_method_addr(klass, method.c_str(), method_args);
	if (!target_addr) return nullptr;

	const auto interceptor = g_hachimi->hachimi_get_interceptor(g_hachimi->hachimi_instance());
	return g_hachimi->interceptor_hook(interceptor, target_addr, destination);
}

void* get_class_from_instance(const void* instance) {
	if (!instance) return nullptr;
	return *static_cast<void* const*>(instance);
}

} // namespace il2cpp
} // namespace gallop
