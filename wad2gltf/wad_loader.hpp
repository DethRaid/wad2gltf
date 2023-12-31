#pragma once

#include <filesystem>

#include "wad.hpp"

// TODO: Return a std::expected with appropriate errors when I get a compiler that handles that well
wad::WAD load_wad_file(const std::filesystem::path& wad_path);
