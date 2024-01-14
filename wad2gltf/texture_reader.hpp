#pragma once

#include <glm/glm.hpp>

#include "wad.hpp"

struct DecodedTexture {
    wad::Name name;
    glm::u16vec2 size;
    std::vector<uint8_t> pixels;
    std::vector<uint8_t> alpha_mask;

    bool has_transparent_pixels() const;
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
DecodedTexture load_texture_from_wad(const wad::Name& texture_name, const wad::WAD& wad);

/**
 * \brief Loads a floor or ceiling flat from the WAD file
 *
 * \param flat_name Name of the flat to load
 * \param wad WAD data to load the flat from
 * \return The loaded flat, without the palette applied
 */
DecodedTexture load_flat_from_wad(const wad::Name& flat_name, const wad::WAD& wad);

/**
 * \brief Loads a sprite from the WAD file
 *
 * Sprites are a bit funky - there may be a few sprites that automatically play in a loop, or sprites that display in
 * response to gameplay events, or different sprites for different viewing angles. Right now this function just loads
 * the base sprite, but eventually it'll load the whole spritesheet
 *
 * \param sprite_name Base name of the sprite to load
 * \param wad WAD data that contains the sprite
 * \return The sprite, without the palette applied
 */
DecodedTexture load_sprite_from_wad(const wad::Name& sprite_name, const wad::WAD& wad);
