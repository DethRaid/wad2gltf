#pragma once

#include <string_view>

#include "wad.hpp"

struct DecodedMapTexture {
    wad::MapTexture info;
    std::vector<uint8_t> pixels;
};

/**
 * Loads a specific texture from a WAD file
 *
 * This function may or may not have a static texture cache. Beware of threading!
 *
 * \param texture_name Name of the texture to load
 * \param wad The WAD file to load the texture from
 * \return The loaded texture, without the palette applied
 * \throws std::runtime_error if there's a runtime error
 */
DecodedMapTexture load_texture_from_wad(const wad::Name& texture_name, const wad::WAD& wad);
