#include "texture_exporter.hpp"

#include <array>

#include <stb_image_write.h>

#include "glm/glm.hpp"

void export_texture(const DecodedTexture& texture, const std::filesystem::path& output_folder, const wad::WAD& wad) {
    // Apply the palette and colormap
    // We export the textures assuming the default palette at full brightness

    const auto palettes_itr = wad.find_lump("PLAYPAL");
    const auto palettes = wad.get_lump_data<std::array<glm::u8vec3, 256>>(*palettes_itr);

    const auto colormaps_itr = wad.find_lump("COLORMAP");
    const auto colormaps = wad.get_lump_data<std::array<uint8_t, 256>>(*colormaps_itr);

    auto pixels = std::vector<glm::u8vec4>{};
    pixels.resize(texture.pixels.size());

    const auto& palette = palettes[0];
    const auto& colormap = colormaps[0];

    for (auto i = 0; i < texture.pixels.size(); i++) {
        const auto color_index = texture.pixels[i];
        const auto mapped_color_index = colormap[color_index];
        const auto color = palette[mapped_color_index];

        const auto alpha_mask = texture.alpha_mask[i];
        pixels[i] = glm::u8vec4{ color, alpha_mask };
    }

    const auto image_file = output_folder / std::format("{}.png", texture.name.to_string());
    const auto image_file_string = image_file.string();
    const auto write_result = stbi_write_png(
        image_file_string.c_str(), texture.width, texture.height, 4, pixels.data(), 0
    );
    if (write_result != 1) {
        throw std::runtime_error{ std::format("Could not write image {}", image_file_string) };
    }
}
