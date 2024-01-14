#include "texture_exporter.hpp"

#include <array>

#include <stb_image_write.h>

#include "glm/glm.hpp"

void export_texture(
    const DecodedTexture& texture, const std::filesystem::path& output_folder, const wad::WAD& wad,
    const MapExtractionOptions& options
) {
    // Apply the palette and colormap
    // We export the textures assuming the default palette at full brightness

    const auto palettes_itr = wad.find_lump("PLAYPAL");
    const auto palettes = wad.get_lump_data<std::array<glm::u8vec3, 256>>(*palettes_itr);

    const auto colormaps_itr = wad.find_lump("COLORMAP");
    const auto colormaps = wad.get_lump_data<std::array<uint8_t, 256>>(*colormaps_itr);

    auto pixels = std::vector<glm::u8vec4>{};
    pixels.resize(texture.pixels.size());

    const auto& palette = palettes[options.palette_index];
    const auto& colormap = colormaps[options.colormap_index];

    for (auto i = 0; i < texture.pixels.size(); i++) {
        auto color_index = texture.pixels[i];
        if (!options.skip_apply_colormap) {
            color_index = colormap[color_index];
        }
        auto color = glm::vec3{ static_cast<float>(color_index) };
        if (!options.skip_apply_palette) {
            color = palette[color_index];
        }

        const auto alpha_mask = texture.alpha_mask[i];
        pixels[i] = glm::u8vec4{color, alpha_mask};
    }

    const auto image_file = output_folder / std::format("{}.png", texture.name.to_string());
    const auto image_file_string = image_file.string();
    const auto write_result = stbi_write_png(
        image_file_string.c_str(), texture.size.x, texture.size.y, 4, pixels.data(), 0
    );
    if (write_result != 1) {
        throw std::runtime_error{std::format("Could not write image {}", image_file_string)};
    }
}
