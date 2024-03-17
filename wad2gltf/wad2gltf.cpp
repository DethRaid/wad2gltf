// wad2gltf.cpp : Defines the entry point for the application.
//

#include <filesystem>
#include <iostream>
#include <string>
#include <ranges>

#include <CLI/CLI.hpp>
#include <fastgltf/core.hpp>
#include <stb_image_write.h>

#include "gltf_export.hpp"
#include "map_reader.hpp"
#include "texture_exporter.hpp"
#include "texture_reader.hpp"
#include "thing_reader.hpp"
#include "wad_loader.hpp"

std::optional<std::string> write_extras(const std::size_t object_index, const fastgltf::Category object_type, void* user_pointer) {
    if(object_type == fastgltf::Category::Nodes) {
        auto* node_extras = static_cast<std::vector<std::optional<std::string>>*>(user_pointer);
        return node_extras->at(object_index);
    }

    return std::nullopt;
}

int main(const int argc, const char** argv) {
    CLI::App app{
        R"(WAD to glTF converter. Extracts maps from a DOOM or DOOM 2 WAD file

This program extracts a map from a DOOM or DOOM 2 IWAD or PWAD file. It generates one glTF Mesh for each sector in the 
map, and one Mesh Primitive for each sector. It can optionally extract the Things from the map, although it places them all at z=0)"
    };

    auto wad_filename = std::filesystem::path{};
    auto extraction_options = MapExtractionOptions{};

    app.add_option("-f,--file", wad_filename, "Name of the WAD file to extract a map from")->required();
    app.add_option("-m,--map", extraction_options.map_name, "Name of the map to extract")->required();
    app.add_option("-o,--output", extraction_options.output_file, "Output the glTF data to this file")->required();
    // app.add_flag("-e,--emission", export_emission_textures, "Generate emission textures by applying the palette for a dimly-lit room. This may or may not yield decent results");
    app.add_flag(
        "-t, --things", extraction_options.export_things,
        "Output the Things from the WAD file. Each Thing will be a Node in the glTF file, with some extras describing the type of Thing"
    );
    app.add_flag(
        "--no-apply-palette", extraction_options.skip_apply_palette,
        "Skip applying a palette to images. The exported images will contain indexes into a color palette, not the colors themselves"
    );
    app.add_option(
        "-p,--palette", extraction_options.palette_index,
        "Index of the palette to use when exporting images. Defaults to 0"
    );
    app.add_flag(
        "--no-apply-colormap", extraction_options.skip_apply_colormap,
        "Skip applying a colormap to images. The exported images will use the original palette indexes"
    );
    app.add_option(
        "-c,--colormap", extraction_options.colormap_index,
        "Index of the colormap to use when exporting images. Defaults to 0"
    );
    app.positionals_at_end();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    try {
        const auto wad = load_wad_file(wad_filename);

        std::cout << std::format("Loaded WAD file {}\n", wad_filename.string());

        auto map = create_mesh_from_map(wad, extraction_options);

        std::cout << std::format("Extracted map {} from WAD\n", extraction_options.map_name);

        load_things_into_map(wad, extraction_options, map);

        // Load all the textures for each sector

        auto [gltf_map, node_extras] = export_to_gltf(extraction_options.map_name, map, extraction_options);
        std::cout << "Generated glTF data\n";

        auto exporter = fastgltf::FileExporter{};
        exporter.setImagePath("textures");
        exporter.setExtrasWriteCallback(write_extras);
        exporter.setUserPointer(&node_extras);

        const auto result = exporter.writeGltfJson(
            gltf_map, extraction_options.output_file, fastgltf::ExportOptions::PrettyPrintJson
        );

        if (result != fastgltf::Error::None) {
            std::cout << std::format("Could not write glTF file: {}\n", fastgltf::getErrorMessage(result));
        } else {
            std::cout << std::format("Wrote glTF to file {}\n", extraction_options.output_file.string());
        }

        const auto images_folder = extraction_options.output_file.parent_path() / "textures";
        std::filesystem::create_directories(images_folder);
        for (const auto& texture : map.textures) {
            export_texture(texture, images_folder, wad, extraction_options);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}
