#pragma once

#include <filesystem>

#include "texture_reader.hpp"

void export_texture(const DecodedTexture& texture, const std::filesystem::path& output_folder, const wad::WAD& wad);
