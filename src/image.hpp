#pragma once

#include "GL/gl.h"
#include "stb_image.h"

namespace gallop {
namespace gui {
struct GallopImage {
	GLuint data = 0;
	int width = 0;
	int height = 0;
};
bool LoadTextureFromMemory(const void* data, size_t data_size, GallopImage& out_texture);
bool LoadTextureFromFile(const char* file_name, GallopImage& out_texture);
bool LoadTextureFromMemoryCompressed(const void* data, size_t data_size, GallopImage& out_texture);
bool LoadTextureFromMemoryCompressedBase85(const char* data, GallopImage& out_texture);
} // namespace gui
} // namespace gallop
