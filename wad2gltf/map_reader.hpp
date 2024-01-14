#pragma once

#include "extraction_options.hpp"
#include "mesh.hpp"
#include "wad.hpp"

/**
 * \file map_reader.hpp
 *
 * Contains utilities to create a mesh from a WAD map
 */


/**
 * Creates a mesh from a map in the WAD data
 *
 * \param wad WAD data that contains the map
 * \param options Options for what data to extract
 * \return A mesh that contains the map
 *
 * \throws std::runtime_error if there's an error at runtime
 *
 * TODO: More options, such as trying to combine faces that use the same texture
 */
Map create_mesh_from_map(const wad::WAD& wad, const MapExtractionOptions& options);
