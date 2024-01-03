#pragma once

#include <cstdint>
#include <array>
#include <optional>

#include <glm/glm.hpp>

#include "texture_reader.hpp"

/**
 * \file mesh.hpp
 *
 * A collection of struct to define meshes for WAD maps
 */

struct Vertex {
    glm::vec3 position;
    glm::vec2 texcoord;
};

struct Face {
    std::array<Vertex, 4> vertices;
    glm::vec3 normal;
    uint32_t texture_index;

    // Indices: 0 1 2 3 2 1
    // Counter-clockwise winding order
};

struct Flat {
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;
    uint32_t texture_index;    
};

struct Sector {
    std::vector<Face> faces;

    Flat ceiling;
    Flat floor;
};

struct Map {
    std::vector<Sector> sectors;

    std::vector<DecodedTexture> textures;

    /**
     * \brief Finds the index of the requested texture in the textures array, or loads it if is doesn't exist
     * \param texture_name Name of the texture to find
     * \return Index of the texture
     */
    uint32_t get_texture_index(const wad::Name& texture_name, const wad::WAD& wad);

    uint32_t get_flat_index(const wad::Name& flat_name, const wad::WAD& wad);
};

inline uint32_t Map::get_texture_index(const wad::Name& texture_name, const wad::WAD& wad) {
    if(!texture_name.is_valid()) {
        throw std::runtime_error{ "Texture name is not valid" };
    }

    for (auto i = 0u; i < textures.size(); i++) {
        const auto& texture = textures[i];
        if (texture.name == texture_name) {
            return i;
        }
    }

    auto index = textures.size();
    textures.emplace_back(load_texture_from_wad(texture_name, wad));
    return static_cast<uint32_t>(index);
}

inline uint32_t Map::get_flat_index(const wad::Name& flat_name, const wad::WAD& wad) {
    if (!flat_name.is_valid()) {
        throw std::runtime_error{ "Texture name is not valid" };
    }

    for (auto i = 0u; i < textures.size(); i++) {
        const auto& texture = textures[i];
        if (texture.name == flat_name) {
            return i;
        }
    }

    auto index = textures.size();
    textures.emplace_back(load_flat_from_wad(flat_name, wad));
    return static_cast<uint32_t>(index);
}
