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

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

struct Face
{
    std::array<Vertex, 4> vertices;
    uint32_t texture_index;

    // Indices: 0 1 2 3 2 1
    // Counter-clockwise winding order
};

struct Sector
{
    std::vector<Face> faces;
};

struct Map {
    std::vector<Sector> sectors;

    std::vector<DecodedMapTexture> textures;

    /**
     * \brief Finds the index of the requested texture in the textures array
     * \param texture_name Name of the texture to find
     * \return Index of the texture, or an empty optional if it doesn't exist
     */
    std::optional<uint32_t> get_texture_index(const wad::Name& texture_name) const;
};

inline std::optional<uint32_t> Map::get_texture_index(const wad::Name& texture_name) const {
    for (auto i = 0; i < textures.size(); i++) {
        const auto& texture = textures[i];
        if(texture.info.name == texture_name) {
            return i;
        }
    }

    return std::nullopt;
}
