#pragma once

#include <vector>
#include <string_view>

#include "mesh.hpp"

/**
 * \file map_reader.hpp
 *
 * Contains utilities to create a mesh from a WAD map
 */

struct WAD;

/**
 * Creates a mesh from a map in the WAD data
 *
 * \param wad WAD data that contains the map
 * \param map_name Name of the map to convert
 * \return A mesh that contains the map
 *
 * \throws std::runtime_error if there's an error at runtime
 *
 * TODO: More options, such as trying to combine faces that use the same texture
 */
std::vector<Face> create_mesh_from_map(const WAD& wad, std::string_view map_name);
