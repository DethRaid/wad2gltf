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

LumpInfo load_lump(const uint8_t* lump_start)
{
    auto lump = LumpInfo{};

    lump.filepos = *reinterpret_cast<const int32_t*>(lump_start);
    lump.size = *reinterpret_cast<const int32_t*>(lump_start + 4);
    memcpy(lump.name, lump_start + 8, 8);

    return lump;
}

WAD load_wad_file(const std::filesystem::path& wad_path)
{
    if(!exists(wad_path))
    {
        throw std::runtime_error{ "Requested WAD file does not exist" };
    }

    // The DOOM and DOOM 2 WADs included in the DOOM 3 BFG edition are 12 and 14 MB, respectively. We can just load the whole thing into RAM without a care
    const auto wad_data = read_binary_file(wad_path);
    if(!wad_data)
    {
        throw std::runtime_error{ "Could not read WAD file" };
    }

    auto* wad_data_ptr = wad_data->data();
    auto wad = WAD{};

    // Fill the header
    wad.identification[0] = *wad_data_ptr;
    wad.identification[1] =*(wad_data_ptr + 1);
    wad.identification[2] =*(wad_data_ptr + 2);
    wad.identification[3] = *(wad_data_ptr + 3);
    wad.numlumps = *reinterpret_cast<const int32_t*>(wad_data_ptr + 4);
    wad.infotableofs = *reinterpret_cast<const int32_t*>(wad_data_ptr + 8);

    wad.lump_directory.resize(wad.numlumps);

    auto* info_table_ptr = wad_data_ptr + wad.infotableofs;
    for(auto i = 0; i < wad.numlumps; i++)
    {
        auto* cur_lump_info_ptr = info_table_ptr + (i * 16);
        const auto lump_data = load_lump(cur_lump_info_ptr);
        std::cout << "Read lump " << lump_data.name << " with offset=" << lump_data.filepos << " and size=" << lump_data.size << "\n";
        wad.lump_directory[i] = lump_data;
    }

    return wad;
}
