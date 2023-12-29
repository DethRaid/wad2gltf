// wad2gltf.cpp : Defines the entry point for the application.
//

#include "wad2gltf.h"

#include <filesystem>

#include "wad_loader.hpp"

/*
 * TODO: Real command-line parsing
 *
 * We want to support --help, but basic usage is that you specify the input file with an optional output file. If no
 * output file is specified, output to stdout
 *
 * We'll also need options to select a map to make a glTF file for, or an option to just make different glTF meshes for each map in the WAD
 */


int main(const int argc, const char** argv)
{
    // if (argc != 2)
    // {
    //     std::cerr << "Invalid arguments. Usage:\n\n\twad2gltf <filename>\n";
    //     return -1;
    // }
    // 
    // const auto arg1 = std::string{argv[1]};
    // 
    // if (arg1 == "--help" || arg1 == "-h" || arg1 == "/?")
    // {
    //     std::cout <<
    //         "wad2gltf: a command-line utility to convert a DOOM WAD file to a glTF 2.0 file\nUsage:\n\n\twad2gltf <filename>\n";
    //     return 0;
    // }
    // 
    // const auto wad_filename = std::filesystem::path{arg1};
    const auto wad_filename = std::filesystem::path{ R"(C:\Program Files (x86)\Steam\steamapps\common\DOOM 3 BFG Edition\base\wads\DOOM.WAD)" };

    try
    {
        const auto wad = load_wad_file(wad_filename);

        std::cout << "WAD file " << wad_filename << "\n\tType:" << wad.identification << "\n\tinfotables:" << wad.infotableofs << "\n\tnumlumps:" << wad.numlumps << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }

    return 0;
}
