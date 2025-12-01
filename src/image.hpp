#pragma once

#include "GL/gl.h"
#include "imgui.h"
#include "stb_image.h"

namespace gallop {
namespace gui {
// Struct for images.
struct GallopImage {
	// Internal data for the image.
	GLuint data = 0;
	// The width of the image.
	int width = 0;
	// The height of the image.
	int height = 0;

	// Returns an ImGui texture ID from the image data.
	inline ImTextureID image() { return (ImTextureID)(intptr_t)data; }
	// Returns the size of the image as an ImVec2 given from the width and height.
	inline ImVec2 size() { return ImVec2(width, height); }
};
bool LoadTextureFromMemory(const void* data, size_t data_size, GallopImage& out_texture);
bool LoadTextureFromFile(const char* file_name, GallopImage& out_texture);
} // namespace gui
} // namespace gallop
