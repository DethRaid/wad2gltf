#pragma once

#include <span>
#include <string_view>

#include <tiny_gltf.h>

#include "mesh.hpp"

/**
 * Exports a map's data to glTF
 *
 * Each Sector becomes a glTF Mesh (and thus a glTF Node). Each face in the Sector is a glTF Primitive. We create a
 * glTF Material for each Texture
 */
tinygltf::Model export_to_gltf(std::string_view name, const Map& map);
