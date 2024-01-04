// wad2gltf.cpp : Defines the entry point for the application.
//

#include <iostream>

#include <filesystem>
#include <string>
#include <ranges>
#include <stb_image_write.h>

#include <CLI/CLI.hpp>

#include "gltf_export.hpp"
#include "map_reader.hpp"
#include "texture_exporter.hpp"
#include "texture_reader.hpp"
#include "wad_loader.hpp"
#include "fastgltf/core.hpp"

/*
 * TODO: Real command-line parsing
 *
 * We want to support --help, but basic usage is that you specify the input file with an optional output file. If no
 * output file is specified, output to stdout
 *
 * We'll also need options to select a map to make a glTF file for, or an option to just make different glTF meshes for
 * each map in the WAD. Could have options for different glTF extensions (basisu) or different ways of combining faces
 * (should faces with the same texture become part of the same mesh?)
 */

int main(const int argc, const char** argv) {
    CLI::App app{
        R"(WAD to glTF converter. Extracts maps from a DOOM or DOOM 2 WAD file

This program extracts a map from a DOOM or DOOM 2 IWAD or PWAD file. It generates one glTF Mesh for each sector in the 
map, and one Mesh Primitive for each sector.)"
    };

    auto wad_filename = std::filesystem::path{};
    auto export_emission_textures = false;
    auto extraction_options = MapExtractionOptions{};
    auto export_options = GltfExportOptions{};

    app.add_option("-f,--file", wad_filename, "Name of the WAD file to extract a map from")->required();
    app.add_option("-m,--map", extraction_options.map_name, "Name of the map to extract")->required();
    app.add_option("-o,--output", export_options.output_file, "Output the glTF data to this file")->required();
    // app.add_flag("-e,--emission", export_emission_textures, "Generate emission textures my applying the palette for a dimly-lit room. This may or may not yield decent results");
    app.add_flag("-t, --things", export_options.export_things, "Output the Things from the WAD file. Each Thing will be a Node in the glTF file, with some extras describing the type of Thing");
    app.positionals_at_end();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    try {
        const auto wad = load_wad_file(wad_filename);

        std::print(std::cout, "Loaded WAD file {}\n", wad_filename.string());

        const auto map = create_mesh_from_map(wad, extraction_options);

        std::print(std::cout, "Extracted map {} from WAD\n", extraction_options.map_name);

        // Load all the textures for each sector

        const auto gltf_map = export_to_gltf(extraction_options.map_name, map, export_options);
        std::print(std::cout, "Generated glTF data\n");

        auto exporter = fastgltf::FileExporter{};
        exporter.setImagePath("textures");

        const auto result = exporter.writeGltfJson(gltf_map, export_options.output_file, fastgltf::ExportOptions::PrettyPrintJson);

        if (result != fastgltf::Error::None) {
            std::print(std::cout, "Could not write glTF file: {}\n", fastgltf::getErrorMessage(result));
        } else {
            std::print(std::cout, "Wrote glTF to file {}\n", export_options.output_file.string());
        }

        const auto images_folder = export_options.output_file.parent_path() / "textures";
        std::filesystem::create_directories(images_folder);
        for (const auto& texture : map.textures) {
            export_texture(texture, images_folder, wad);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}
