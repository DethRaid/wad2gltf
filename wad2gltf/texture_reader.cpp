#include "texture_reader.hpp"

#include <iostream>
#include <map>
#include <stb_image_write.h>
#include <unordered_map>

#include "wad.hpp"
#include "glm/detail/qualifier.hpp"

struct DecodedPatch {
    wad::PatchHeader header;
    std::vector<uint8_t> pixel_data;
    std::vector<uint8_t> transparency;
};

static std::unordered_map<wad::Name, DecodedPatch> patch_cache;

const DecodedPatch& get_patch(const wad::Name& patch_name, const wad::WAD& wad) {
    const auto patch_itr = wad.find_lump(patch_name);
    const auto* patch_ptr = wad.raw_data.data() + patch_itr->filepos;
    const auto* patch_header = reinterpret_cast<const wad::PatchHeader*>(patch_ptr);
    const auto* column_offsets = &patch_header->column_offsets_start;

    if (const auto itr = patch_cache.find(patch_name); itr != patch_cache.end()) {
        return itr->second;
    }

    auto patch = DecodedPatch{.header = *patch_header};
    patch.pixel_data.resize(patch_header->width * patch_header->height);
    patch.transparency.resize(patch.pixel_data.size(), 0);

    for (auto i = 0; i < patch_header->width; i++) {
        auto* read_ptr = patch_ptr + column_offsets[i];

        do {
            // Row where the post begins
            const auto post_row = *read_ptr;
            read_ptr++;
            // Height of the post - aka number of rows it covers
            const auto post_height = *read_ptr;
            read_ptr++;

            // Padding
            read_ptr++;

            for (auto j = 0; j < post_height; j++) {
                const auto write_y = j + post_row;
                const auto write_x = i;
                if (write_y >= patch_header->height || write_x >= patch_header->width) {
                    read_ptr++;
                    continue;
                }
                const auto write_idx = write_y * patch_header->width + write_x;
                patch.pixel_data[write_idx] = *read_ptr;
                patch.transparency[write_idx] = 0xFF;
                read_ptr++;
            }

            // More padding
            read_ptr++;
        } while (*read_ptr != 255);
    }

    const auto filename = std::format("textures/patches/{}.png", patch_name);
    stbi_write_png(filename.c_str(), patch_header->width, patch_header->height, 1, patch.pixel_data.data(), 0);

    const auto itr = patch_cache.emplace(
        patch_name, std::move(patch)
    );
    return itr.first->second;
}

const wad::MapTexture* find_texture_in_texture_group(
    const wad::Name& texture_name, const std::string_view group_name, const wad::WAD& wad
) {
    const auto texture1_itr = wad.find_lump("TEXTURE1");

    const auto* texture1_ptr = wad.raw_data.data() + texture1_itr->filepos;
    const auto* texture1 = reinterpret_cast<const wad::Texture1*>(texture1_ptr);

    const auto* map_textures_offsets = &texture1->offset_array_start;

    // Figure out where the requested texture is
    for (auto i = 0u; i < texture1->numtextures; i++) {
        const auto offset = map_textures_offsets[i];
        const auto* test_texture = reinterpret_cast<const wad::MapTexture*>(texture1_ptr + offset);
        if (test_texture->name == texture_name) {
            return test_texture;
        }
    }

    return nullptr;
}

std::vector<uint8_t> load_flat_from_wad(const wad::Name& texture_name, const wad::WAD& wad) {
    try {
        const auto itr = wad.find_lump(texture_name);
        const auto pixels = wad.get_lump_data<uint8_t>(*itr);
        return std::vector(pixels.begin(), pixels.end());
    } catch (const std::exception& e) {
        // The texture is not a flat, return null or something
        return {};
    }
}

DecodedTexture load_texture_from_wad(const wad::Name& texture_name, const wad::WAD& wad) {
    const auto* map_texture = find_texture_in_texture_group(texture_name, "TEXTURE1", wad);
    if (map_texture == nullptr) {
        map_texture = find_texture_in_texture_group(texture_name, "TEXTURE2", wad);
    }

    if (map_texture == nullptr) {
        throw std::runtime_error{std::format("Could not find requested texture {}", texture_name)};
    }

    auto pixels = std::vector<uint8_t>(map_texture->width * map_texture->height);
    auto transparency = std::vector<uint8_t>(pixels.size());

    const auto patch_names_itr = wad.find_lump("PNAMES");
    auto* patch_names_ptr = wad.raw_data.data() + patch_names_itr->filepos;
    const auto num_patches = *reinterpret_cast<const uint32_t*>(patch_names_ptr);
    patch_names_ptr += sizeof(uint32_t);

    const auto patch_names = std::span{reinterpret_cast<const wad::Name*>(patch_names_ptr), num_patches};

    const auto* patches = &map_texture->patches_array_start;
    for (auto patch_index = 0; patch_index < map_texture->patchcount; patch_index++) {
        const auto& map_patch = patches[patch_index];
        const auto& patch_name = patch_names[map_patch.patch];

        const auto patch = get_patch(patch_name, wad);

        // Copy patch data to the map texture
        for (auto patch_y = 0; patch_y < patch.header.height; patch_y++) {
            for (auto patch_x = 0; patch_x < patch.header.width; patch_x++) {
                const auto x = map_patch.origin_x + patch_x;
                const auto y = map_patch.origin_y + patch_y;

                if (x < 0 || y < 0 || x >= map_texture->width || y >= map_texture->height) {
                    // Possibly?
                    continue;
                }

                const auto read_idx = patch_x + patch_y * patch.header.width;
                const auto write_idx = x + y * map_texture->width;
                pixels[write_idx] = patch.pixel_data[read_idx];
                transparency[write_idx] = patch.transparency[read_idx];
            }
        }
    }

    return DecodedTexture{
        .name = map_texture->name, .width = map_texture->width, .height = map_texture->height,
        .pixels = std::move(pixels), .alpha_mask = std::move(transparency)
    };
}
