#include "texture_reader.hpp"

#include "wad.hpp"

void load_texture_from_wad(const std::string_view texture_name, const wad::WAD& wad)
{
    const auto texture1_itr = wad.find_lump("TEXTURE1");

    const auto* texture1_ptr = wad.raw_data.data() + texture1_itr->filepos;
    const auto* texture1 = reinterpret_cast<const wad::Texture1*>(texture1_ptr);

    const auto* map_textures_offsets = &texture1->offset_array_start;
    const auto* map_textures_ptr = texture1_ptr + (texture1->numtextures + 1) * sizeof(uint32_t);

    const wad::MapTexture* map_texture = nullptr;
    // Figure out where the requested texture is
    for(auto i = 0; i < texture1->numtextures; i++)
    {
        const auto offset = map_textures_offsets[i];
        const auto* test_texture = reinterpret_cast<const wad::MapTexture*>(map_textures_ptr + offset);
        if(memcmp(test_texture->name, texture_name.data(), texture_name.size()) == 0)
        {
            map_texture = test_texture;
            break;
        }
    }

    if(map_texture == nullptr)
    {
        throw std::runtime_error{ std::format("Could not find requested texture {}", texture_name) };
    }

    auto paletted_pixels = std::vector<uint8_t>{};
    paletted_pixels.resize(map_texture->width * map_texture->height);

    const auto patch_names_itr = wad.find_lump("PNAMES");
    auto* patch_names_ptr = wad.raw_data.data() + patch_names_itr->filepos;
    const auto num_patches = *reinterpret_cast<const uint32_t*>(patch_names_ptr);
    patch_names_ptr += sizeof(uint32_t);

    const auto patch_names = std::span{ reinterpret_cast<const wad::Name*>(patch_names_ptr), num_patches };

    const auto* patches = &map_texture->patches_array_start;
    for(auto i = 0; i < map_texture->patchcount; i++)
    {
        const auto& map_patch = patches[i];
        const auto& patch_name = patch_names[map_patch.patch];
        const auto patch_itr = wad.find_lump(patch_name.val);
        const auto* patch_ptr = wad.raw_data.data() + patch_itr->filepos;
        const auto* patch = reinterpret_cast<const wad::PatchHeader*>(patch_ptr);
        const auto* column_offsets = &patch->column_offsets_start;

    }
}
