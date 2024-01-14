#pragma once

#include <filesystem>
#include <string>

 /**
  * \brief Options for how to extract a map
  */
struct MapExtractionOptions {
    /**
     * \brief Name of the map to extract
     */
    std::string map_name = "E1M1";

    /**
     * \brief Whether or not to extract the THINGS in the map
     */
    bool export_things = true;

    /**
     * \brief Path to output the glTF file to
     */
    std::filesystem::path output_file;

    /**
     * \brief Whether or not to apply the palette when exporting images
     */
    bool skip_apply_palette = false;

    /**
     * \brief Index of the palette to apply when exporting textures
     */
    uint32_t palette_index = 0;

    /**
     * \brief Whether or not to apply a colormap when exporting images
     */
    bool skip_apply_colormap = false;

    /**
     * \brief Index of the colormap to apply when exporting textures
     */
    uint32_t colormap_index = 0;
};
