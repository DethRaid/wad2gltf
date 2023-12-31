#pragma once

#include <span>
#include <string_view>

#include <tiny_gltf.h>

#include "mesh.hpp"

/**
 * Exports a map's data to glTF
 */
tinygltf::Model export_to_gltf(std::string_view name, std::span<const Face> faces);
