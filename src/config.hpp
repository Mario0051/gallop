#pragma once

#include "toml.hpp"
#include <nlohmann/detail/macro_scope.hpp>

#define TOML11_FIND_MEMBER_VARIABLE_FROM_VALUE_WITH_DEFAULT(VAR_NAME)                                                                                          \
	gallop::toml_funcs::find_member_variable_from_value(obj.VAR_NAME, v, TOML11_STRINGIZE(VAR_NAME));

#define TOML11_ASSIGN_MEMBER_VARIABLE_TO_VALUE_WITH_DEFAULT(VAR_NAME)                                                                                          \
	gallop::toml_funcs::assign_member_variable_to_value(obj.VAR_NAME, v, TOML11_STRINGIZE(VAR_NAME));

#define TOML11_DEFINE_CONVERSION_NON_INTRUSIVE_WITH_DEFAULT(NAME, ...)                                                                                         \
	namespace toml {                                                                                                                                           \
	template <> struct from<NAME> {                                                                                                                            \
		template <typename TC> static NAME from_toml(const basic_value<TC>& v)                                                                                 \
		{                                                                                                                                                      \
			NAME obj;                                                                                                                                          \
			TOML11_FOR_EACH_VA_ARGS(TOML11_FIND_MEMBER_VARIABLE_FROM_VALUE_WITH_DEFAULT, __VA_ARGS__)                                                          \
			return obj;                                                                                                                                        \
		}                                                                                                                                                      \
	};                                                                                                                                                         \
	template <> struct into<NAME> {                                                                                                                            \
		template <typename TC> static basic_value<TC> into_toml(const NAME& obj)                                                                               \
		{                                                                                                                                                      \
			toml::basic_value<TC> v = typename toml::basic_value<TC>::table_type{};                                                                            \
			TOML11_FOR_EACH_VA_ARGS(TOML11_ASSIGN_MEMBER_VARIABLE_TO_VALUE_WITH_DEFAULT, __VA_ARGS__)                                                          \
			return v;                                                                                                                                          \
		}                                                                                                                                                      \
	};                                                                                                                                                         \
	} /* toml */

namespace gallop {

int init_config();
int save_config();

typedef struct gallop_char_info_s {
	int charaId = 0;
	int clothId = 0;
	bool replaceMini = false;
	bool homeScreenOnly = false;
	// TOML11_INTO_TOML(gallop_char_info_s, charaId, clothId, replaceMini, homeScreenOnly)
	// TOML11_FROM_TOML(gallop_char_info_s, charaId, clothId, replaceMini, homeScreenOnly)
} gallop_char_info_t;

typedef struct gallop_config_s {
	bool discordRPC = true;
	std::unordered_map<std::string, gallop_char_info_t> replaceCharacters;
	// TOML11_INTO_TOML(gallop_config_s, replaceCharacters)
	// TOML11_FROM_TOML(gallop_config_s, replaceCharacters)
} gallop_config_t;

extern gallop_config_t conf;

// TOML related functions (pertaining to deserialization)
namespace toml_funcs {
template <typename T, typename TC> void find_member_variable_from_value(T& obj, const toml::basic_value<TC>& v, const char* var_name)
{
	obj = toml::find_or_default<T>(v, var_name);
}

template <typename T, typename TC> void assign_member_variable_to_value(const T& obj, toml::basic_value<TC>& v, const char* var_name) { v[var_name] = obj; }
} // namespace toml_funcs
} // namespace gallop

TOML11_DEFINE_CONVERSION_NON_INTRUSIVE_WITH_DEFAULT(gallop::gallop_char_info_s, charaId, clothId, replaceMini, homeScreenOnly)
TOML11_DEFINE_CONVERSION_NON_INTRUSIVE_WITH_DEFAULT(gallop::gallop_config_s, discordRPC, replaceCharacters)
