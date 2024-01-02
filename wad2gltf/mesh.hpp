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

struct Sector {
    std::vector<Face> faces;
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
};

inline uint32_t Map::get_texture_index(const wad::Name& texture_name, const wad::WAD& wad) {
    if(!texture_name.is_valid()) {
        throw std::runtime_error{ "Texture name is not valid" };
    }
    for (auto i = 0; i < textures.size(); i++) {
        const auto& texture = textures[i];
        if (texture.name == texture_name) {
            return i;
        }
    }

    auto index = textures.size();
    textures.emplace_back(load_texture_from_wad(texture_name, wad));
    return static_cast<uint32_t>(index);
}
