#pragma once
#include <string_view>

namespace wad
{
    struct Wad;
}

/**
 * Loads a specific texture from a WAD file
 *
 * This function may or may not have a static texture cache. Beware of threading!
 *
 * \param texture_name Name of the texture to load
 * \param wad The WAD file to load the texture from
 * \return The loaded texture, with the default palette
 * \throws std::runtime_error if there's a runtime error
 */
void load_texture_from_wad(std::string_view texture_name, const wad::Wad& wad);
