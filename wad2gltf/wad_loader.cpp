#include "wad_loader.hpp"

#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

std::optional<std::vector<uint8_t>> read_binary_file(const std::filesystem::path& filename) {
    const auto& filename_string = filename.string();
    auto* file = fopen(filename_string.c_str(), "rb");
    if (file == nullptr) {
        return std::nullopt;
    }

    fseek(file, 0, SEEK_END);
    const auto file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    auto file_content = std::vector<uint8_t>(file_size);
    fread(file_content.data(), 1, file_size, file);

    fclose(file);

    return file_content;
}

int32_t read_int(const uint8_t* read_ptr)
{
    auto value = int32_t{};
    value |= *read_ptr << 24;
    read_ptr++;
    value |= (*read_ptr) << 16;
    read_ptr++;
    value |= (*read_ptr) << 8;
    read_ptr++;
    value |= (*read_ptr);

    return value;
}

wad::LumpInfo load_lump(const uint8_t* lump_start)
{
    auto lump = wad::LumpInfo{};

    lump.filepos = *reinterpret_cast<const int32_t*>(lump_start);
    lump.size = *reinterpret_cast<const int32_t*>(lump_start + 4);
    memcpy(lump.name, lump_start + 8, 8);

    return lump;
}

wad::WAD load_wad_file(const std::filesystem::path& wad_path)
{
    if(!exists(wad_path))
    {
        throw std::runtime_error{ "Requested WAD file does not exist" };
    }

    // The DOOM and DOOM 2 WADs included in the DOOM 3 BFG edition are 12 and 14 MB, respectively. We can just load the whole thing into RAM without a care
    auto wad_data = read_binary_file(wad_path);
    if(!wad_data)
    {
        throw std::runtime_error{ "Could not read WAD file" };
    }

    auto wad = wad::WAD{};
    wad.raw_data = std::move(*wad_data);

    auto* wad_data_ptr = wad.raw_data.data();

    wad.header = reinterpret_cast<wad::Header*>(wad_data_ptr);
    auto* lump_directory_ptr = reinterpret_cast<wad::LumpInfo*>(wad_data_ptr + wad.header->infotableofs);
    wad.lump_directory = std::span{ lump_directory_ptr, lump_directory_ptr + wad.header->numlumps };
    
    return wad;
}
