#pragma once

#include <string_view>

#include <fastgltf/types.hpp>

#include "extraction_options.hpp"
#include "mesh.hpp"

struct ExportedWad {
    fastgltf::Asset asset;
    std::vector<std::optional<std::string>> node_extras;
};

/**
 * Exports a map's data to glTF
 *
 * Each Sector becomes a glTF Mesh (and thus a glTF Node). Each face in the Sector is a glTF Primitive. We create a
 * glTF Material for each Texture
 */
ExportedWad export_to_gltf(std::string_view name, const Map& map, const MapExtractionOptions& options);
