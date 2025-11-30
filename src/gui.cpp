#include "backend/imgui_impl_opengl3.h"
#include "backend/imgui_impl_win32.h"
#include "config.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "mdb.hpp"
#include <fmt/base.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "gallop.hpp"
#include <GL/gl.h>
#include <functional>
#include <tchar.h>
#include <windows.h>

constexpr int window_width = 700;
constexpr int window_height = 500;

// Data stored per platform window
struct WGL_WindowData {
	HDC hDC;
};

// Data
static HGLRC g_hRC;
static WGL_WindowData g_MainWindow;
static int g_Width;
static int g_Height;

// Forward declarations of helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
void ResetDeviceWGL();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Import fonts
#include "mplus1code.cpp"
// Google is there a need for 2 separate fonts for symbols
#include "notosans_symbols.cpp"
#include "notosans_symbols2.cpp"
// Lucide for icons
#include "lucide.cpp" // Don't ask.
#include "lucide.h"

float ImGuiButtonWidths(const std::vector<std::string>& buttons)
{
	ImGuiStyle& style = ImGui::GetStyle();
	float width = 0.0f;
	for (const auto& item : buttons) {
		width += ImGui::CalcTextSize(item.c_str()).x + (style.FramePadding.x * 2.f) + style.ItemSpacing.x;
	}
	width -= style.ItemSpacing.x;
	return width;
}

#define IMGUI_RIGHT_ALIGNED_BUTTONS(func, ...)                                                                                                                 \
	{                                                                                                                                                          \
		ImGuiStyle& style = ImGui::GetStyle();                                                                                                                 \
		std::vector<std::string> arr = {__VA_ARGS__};                                                                                                          \
		float width = ::ImGuiButtonWidths(arr);                                                                                                                \
		auto f = func;                                                                                                                                         \
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - width + (style.FramePadding.x));                                      \
		for (size_t i = 0; i < arr.size(); i++) {                                                                                                              \
			std::string item = arr[i];                                                                                                                         \
			if (ImGui::Button(item.c_str())) {                                                                                                                 \
				f(i);                                                                                                                                          \
			}                                                                                                                                                  \
			ImGui::SameLine();                                                                                                                                 \
		}                                                                                                                                                      \
	}

namespace gallop {
bool show_demo_window = true;
bool show_another_window = false;
bool done = false;

ImVec4 clear_color;

HWND hwnd;
ImGuiIO io;
WNDCLASSEXW wc;

namespace gui {
int init()
{
	// Make process DPI aware and obtain main monitor scale
	ImGui_ImplWin32_EnableDpiAwareness();
	float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

	// Create application window
	WNDCLASSEXW wc = {sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"gallop", nullptr};
	::RegisterClassExW(&wc);
	HWND parent = ::FindWindow(nullptr, L"UmamusumePrettyDerby_Jpn");
	HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"CarrotJuicer", WS_POPUPWINDOW | WS_CAPTION, 100, 100, (int)(window_width * main_scale),
								(int)(window_height * main_scale), parent, nullptr, wc.hInstance, nullptr);

	HMENU hMenu = ::GetSystemMenu(hwnd, FALSE);
	if (hMenu) {
		::EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	// Initialize OpenGL
	if (!CreateDeviceWGL(hwnd, &g_MainWindow)) {
		CleanupDeviceWGL(hwnd, &g_MainWindow);
		::DestroyWindow(hwnd);
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}
	wglMakeCurrent(g_MainWindow.hDC, g_hRC);

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.IniFilename = NULL;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsClassic();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for
									 // dynamic style scaling, changing this requires resetting
									 // Style + calling this again)
	style.FontScaleDpi = main_scale; // Set initial font scale. (using
									 // io.ConfigDpiScaleFonts=true makes this unnecessary. We
									 // leave both here for documentation purpose)

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_InitForOpenGL(hwnd);
	ImGui_ImplOpenGL3_Init();

	spdlog::info("[gallop] Successfully initialized ImGUI interface!");

	ImFontConfig config;
	// config.GlyphOffset.y = -1;
	config.MergeMode = true;
	style.FontSizeBase = 18.0f;
	//  io.Fonts->AddFontDefault();
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(m_plus_1_code_compressed_data_base85);
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(notosans_symbols_compressed_data_base85, 0.0f, &config);
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(notosans_symbols2_compressed_data_base85, 0.0f, &config);
	config.GlyphMinAdvanceX = style.FontSizeBase;
	config.GlyphOffset.y = 3.0f;
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(lucide_compressed_data_base85, 0.0f, &config);

	// Our state
	show_demo_window = true;
	show_another_window = false;
	clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	return 0;
}

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled(ICON_LC_CIRCLE_QUESTION_MARK);
	if (ImGui::BeginItemTooltip()) {
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

bool TreeNodeWithWidth(const char* label, ImGuiTreeNodeFlags flags = 0, float width = 0.0f)
{
	if (width == 0.0f)
		return ImGui::TreeNodeEx(label, flags);

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGui::SetNextItemWidth(width);
	float backup_work_max_x = window->WorkRect.Max.x;
	window->WorkRect.Max.x = window->DC.CursorPos.x + ImGui::CalcItemWidth();
	bool ret = ImGui::TreeNodeEx(label, flags);
	window->WorkRect.Max.x = backup_work_max_x;
	return ret;
}

// This is horrid but I am never making this a macro that would suck even more
template <typename T, typename U>
void ImGuiComboFromDictionaryWithFilter(std::string label, std::string preview_value, std::map<T, U> dict, std::function<std::string(T, U)> value_name_func,
										std::function<bool(T, U)> selected_cond, std::function<void(T, U)> selection_func, ImGuiComboFlags flags = 0)
{
	if (ImGui::BeginCombo(label.c_str(), preview_value.c_str(), flags)) {
		static ImGuiTextFilter filter;
		if (ImGui::IsWindowAppearing()) {
			ImGui::SetKeyboardFocusHere();
			filter.Clear();
		}
		ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
		filter.Draw("##Filter", -FLT_MIN);
		for (const auto& [k, v] : dict) {
			std::string name = value_name_func(k, v);
			bool selected = selected_cond(k, v);
			if (filter.PassFilter(name.c_str())) {
				if (ImGui::Selectable(name.c_str(), selected)) {
					selection_func(k, v);
				}
			}
		}
		ImGui::EndCombo();
	}
}

int update_and_paint()
{
	// yes
	// Poll and handle messages (inputs, window resize, etc.)
	// See the WndProc() function below for our to dispatch events to the Win32
	// backend.
	MSG msg;
	while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			done = true;
	}
	if (done)
		return 0;
	if (::IsIconic(hwnd)) {
		::Sleep(10);
		return 0;
	}

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::SetNextWindowPos({0, 0});
	ImGuiStyle& style = ImGui::GetStyle();
	if (ImGui::Begin("libgallop", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration)) {
		if (ImGui::BeginTabBar("Main", ImGuiTabBarFlags_None)) {
			if (ImGui::BeginTabItem("Log")) {
				sink->Draw();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Configuration")) {
				static int newChar = 0;
				std::vector<std::string> removed;
#define FORMAT_GET_OR_DEFAULT(dict, i) fmt::format("{} ({})", dict.contains(i) ? dict.at(i) : "???", i).c_str()
				if (ImGui::CollapsingHeader("Override Characters")) {
					for (auto& item : gallop::conf.replaceCharacters) {
						if (ImGui::BeginPopup("newID")) {
							ImGuiComboFromDictionaryWithFilter<int, std::string>(
								"Character ID", FORMAT_GET_OR_DEFAULT(id2name, std::stoi(item.first)), id2name,
								[&](int key, std::string value) { return fmt::format("{} ({})", value, key); },
								[&](int key, std::string value) {
									(void)value;
									return std::stoi(item.first) == key;
								},
								[&](int key, std::string value) {
									(void)value;
									auto i = gallop::conf.replaceCharacters.extract(item.first);
									i.key() = std::to_string(key);
									gallop::conf.replaceCharacters.insert(std::move(i));
								});
							ImGui::EndPopup();
						}
						// Buttons n such
						bool success = TreeNodeWithWidth(FORMAT_GET_OR_DEFAULT(id2name, std::stoi(item.first)), ImGuiTreeNodeFlags_Framed,
														 -ImGuiButtonWidths({ICON_LC_PENCIL, ICON_LC_X}));
						ImGui::SameLine();
						ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, style.ItemSpacing.x / 2.f);
						IMGUI_RIGHT_ALIGNED_BUTTONS(
							[&](int idx) {
								switch (idx) {
								case 0:
									ImGui::OpenPopup("newID");
									break;
								case 1:
									// Done later to prevent massive bad things from happening
									removed.push_back(item.first);
									break;
								}
							},
							ICON_LC_PENCIL, ICON_LC_X);
						ImGui::PopStyleVar();
						ImGui::NewLine();
						if (success) {
							ImGuiComboFromDictionaryWithFilter<int, std::string>(
								"New Character ID", FORMAT_GET_OR_DEFAULT(id2name, item.second.charaId), id2name,
								[&](int key, std::string value) { return fmt::format("{} ({})", value, key); },
								[&](int key, std::string value) {
									(void)value;
									return item.second.charaId == key;
								},
								[&](int key, std::string value) {
									(void)value;
									item.second.charaId = key;
								});
							ImGui::SameLine();
							HelpMarker("Setting this to None will disable character replacing.");
							ImGuiComboFromDictionaryWithFilter<int, std::string>(
								"New Dress ID", FORMAT_GET_OR_DEFAULT(id2dress, item.second.clothId), id2dress,
								[&](int key, std::string value) { return fmt::format("{} ({})", value, key); },
								[&](int key, std::string value) {
									(void)value;
									return item.second.clothId == key;
								},
								[&](int key, std::string value) {
									(void)value;
									item.second.clothId = key;
								});
							ImGui::SameLine();
							HelpMarker("Setting this to Default will use the outfit already specified, if available.");
							ImGui::Checkbox("Replace Mini Model", &item.second.replaceMini);
							ImGui::Checkbox("Home Screen Only", &item.second.homeScreenOnly);
							ImGui::TreePop();
						}
					}

					for (const auto& tbr : removed) {
						gallop::conf.replaceCharacters.erase(tbr);
					}

					// IMGUI_RIGHT_ALIGNED_BUTTONS("X")

					if (ImGui::Button("New Override...")) {
						newChar = 0;
						ImGui::OpenPopup("NewReplaceCharacter");
					}
					if (ImGui::BeginPopup("NewReplaceCharacter")) {
						ImGuiComboFromDictionaryWithFilter<int, std::string>(
							"Character ID", FORMAT_GET_OR_DEFAULT(id2name, newChar), id2name,
							[&](int key, std::string value) { return fmt::format("{} ({})", value, key); },
							[&](int key, std::string value) {
								(void)value;
								return newChar == key;
							},
							[&](int key, std::string value) {
								(void)value;
								newChar = key;
							});
						if (ImGui::Button("Add Override")) {
							gallop::conf.replaceCharacters.emplace(std::to_string(newChar), std::forward<gallop_char_info_s>({}));
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
				}
			end_early:
				ImGui::Separator();
				if (ImGui::Button("Load Config")) {
					gallop::init_config();
				}
				ImGui::SameLine();
				if (ImGui::Button("Save Config")) {
					gallop::save_config();
				}
#undef FORMAT_GET_OR_DEFAULT
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	// ImGui::ShowMetricsWindow();

	// Rendering
	ImGui::Render();
	glViewport(0, 0, g_Width, g_Height);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Present
	::SwapBuffers(g_MainWindow.hDC);

	return 0;
}

int destroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceWGL(hwnd, &g_MainWindow);
	wglDeleteContext(g_hRC);
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}

int run()
{
	int ret = 0;
	ret = init();
	if (ret)
		return 1;
	while (!done) {
		update_and_paint();
		if (done)
			break;
	}
	destroy();

	return 0;
}
} // namespace gui
} // namespace gallop

// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
	HDC hDc = ::GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd = {0};
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

	const int pf = ::ChoosePixelFormat(hDc, &pfd);
	if (pf == 0)
		return false;
	if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
		return false;
	::ReleaseDC(hWnd, hDc);

	data->hDC = ::GetDC(hWnd);
	if (!g_hRC)
		g_hRC = wglCreateContext(data->hDC);
	return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
	wglMakeCurrent(nullptr, nullptr);
	::ReleaseDC(hWnd, data->hDC);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if
// dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your
// main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to
// your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from
// your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED) {
			g_Width = LOWORD(lParam);
			g_Height = HIWORD(lParam);
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
