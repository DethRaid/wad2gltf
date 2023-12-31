#pragma once

#include <cstdint>
#include <array>

#include <glm/glm.hpp>

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
