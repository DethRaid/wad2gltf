#include "texture_reader.hpp"

#include "wad.hpp"

void load_texture_from_wad(const std::string_view texture_name, const wad::WAD& wad)
{
    const auto texture1_itr = wad.find_lump("TEXTURE1");

    const auto* texture1_ptr = wad.raw_data.data() + texture1_itr->filepos;
    const auto* texture1 = reinterpret_cast<const wad::Texture1*>(texture1_ptr);

    const auto* map_textures_offsets = &texture1->offset_array_start;
    const auto* map_textures_ptr = texture1_ptr + (texture1->numtextures + 1) * sizeof(uint32_t);

    auto requested_texture_offset = -1;
    // Figure out where the requested texture is
    for(auto i = 0; i < texture1->numtextures; i++)
    {
        const auto offset = map_textures_offsets[i];
        const auto* map_texture = reinterpret_cast<const wad::MapTexture*>(map_textures_ptr + offset);
        if(memcmp(map_texture->name, texture_name.data(), texture_name.size()) == 0)
        {
            requested_texture_offset = i;
            break;
        }
    }

    if(requested_texture_offset == -1)
    {
        throw std::runtime_error{ std::format("Could not find requested texture {}", texture_name) };
    }

    TODO: All the other stuff
}
