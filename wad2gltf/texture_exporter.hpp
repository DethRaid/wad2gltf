#pragma once

#include <filesystem>

#include "extraction_options.hpp"
#include "texture_reader.hpp"

void export_texture(
    const DecodedTexture& texture, const std::filesystem::path& output_folder, const wad::WAD& wad,
    const MapExtractionOptions& options
);
