#pragma once

#include <cstdint>
#include <format>
#include <vector>
#include <span>
#include <stdexcept>

namespace wad {
    struct Header
    {
        /**
         * Identifier for the file. Must be either PWAD or IWAD
         */
        char identification[4] = {'I', 'W', 'A', 'D'};
        /**
         * Number of lumps in this WAD
         */
        int32_t numlumps = 0;
        /**
         * Offset of the info table in the WAD data
         */
        int32_t infotableofs = 0;
    };

    struct LumpInfo
    {
        /**
         * Byte offset of the lump's data in the file
         */
        int32_t filepos = 0;

        /**
         * Size of the lump, in bytes
         */
        int32_t size = 0;

        /**
         * Name of the lump, hopefully null-terminated
         */
        char name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    };

    /**
     * WAD - Where's All the Data?
     *
     * File format of DOOM and related games
     *
     * Currently implements the DOOM file format, but not any successors. That might come later
     *
     * All the pointers in this data structure refer to the raw data vector. Thus, copying this data
     * structure is not allowed. Maybe one day I'll write a good copy constructor/operator
     */
    struct WAD
    {
        Header* header = nullptr;

        std::span<LumpInfo> lump_directory;

        /**
         * Raw data read from the WAD file
         */
        std::vector<uint8_t> raw_data;

        WAD() = default;

        WAD(const WAD& other) = delete;
        WAD& operator=(const WAD& other) = delete;

        WAD(WAD&& old) noexcept = default;
        WAD& operator=(WAD&& old) noexcept = default;

        auto find_lump(const std::string_view lump_name) const
        {
            const auto itr = std::ranges::find_if(lump_directory, [&](const wad::LumpInfo& lump)
            {
                return memcmp(lump.name, lump_name.data(), lump_name.size()) == 0;
            });
            if (itr == lump_directory.end())
            {
                throw std::runtime_error{ std::format("Could not find requested map {}", lump_name) };
            }

            return itr;
        }

        template <typename LumpDataType>
        std::span<const LumpDataType> get_lump_data(const wad::LumpInfo& lump) const
        {
            const auto* lump_data_ptr = reinterpret_cast<const LumpDataType*>(raw_data.data() + lump.filepos);
            return std::span{ lump_data_ptr, static_cast<size_t>(lump.size) / sizeof(LumpDataType) };
        }
    };

    struct Vertex
    {
        int16_t x = 0;
        int16_t y = 0;
    };

    struct LineDef
    {
        uint16_t start_vertex = 0;
        uint16_t end_vertex = 0;
        int16_t flags = 0;
        int16_t special_type = 0;
        int16_t sector_tag = 0;
        int16_t front_sidedef = 0;
        int16_t back_sidedef = 0;
    };

    struct SideDef
    {
        int16_t x_offset = 0;
        int16_t y_offset = 0;
        char upper_texture_name[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        char lower_texture_name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        char middle_texture_name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        int16_t sector_number = 0;
    };

    struct Sector
    {
        int16_t floor_height = 0;
        int16_t ceiling_height = 0;
        char floor_texture[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        char ceiling_texture[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        int16_t light_level = 0;
        int16_t special_type = 0;
        int16_t tag_number = 0;
    };

    struct Texture1
    {
        /**
         * Number of textures in this lump
         */
        uint32_t numtextures = 0;

        /**
         * Start of an array of the offset of each map texture within this lump
         *
         * Map textures might have different sizes if they're made of different numbers of patches, so we need this
         * offset list to know where each one begins
         *
         * The offset array is not represented in this struct. Rather, we have a variable that'll have the memory
         * address of the start of this array. WAD readers should reinterpret this variable as an integer array instead
         * of using it directly
         */
        uint32_t offset_array_start = 0;

        /**
         * After the array of offsets is the array of map textures. I don't think there's a good way to represent that
         * in this struct, so the client code will have to try its best
         */
    };

    /**
     * Struct defining how to place a patch inside a texture
     */
    struct MapPatch
    {
        /**
         * Offset of the patch relative to the upper-left of the texture
         */
        int16_t origin_x = 0;

        /**
         * Offset of the patch relative to the upper-left of the texture
         */
        int16_t origin_y = 0;

        /**
         * Index into the patch names array (PNAMES lump) for which patch to draw here
         */
        int16_t patch = 0;

        /**
         * Whether or not to mirror the patch. Unused
         */
        int16_t stepdir = 0;

        /**
         * Override colormap to draw this patch with. Unused
         */
        int16_t colormap = 0;
    };

    struct MapTexture
    {
        /**
         * Name of this map texture
         */
        char name[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

        /**
         * Defines if the texture is masked? DOOM Wiki implies this isn't used
         */
        uint32_t masked = 0;

        /**
         * Height of the map texture
         */
        uint16_t width = 0;

        /**
         * Width of the map texture
         */
        uint16_t height = 0;

        /**
         * Obsolete
         */
        uint32_t columndirectory;

        /**
         * Number of patches in this texture
         */
        uint16_t patchcount;

        /**
         * Start of an array of the patches that this texture uses. Like offset_array_start in Texture1, this is a
         * variable-length array that's difficult to express statically
         */
        MapPatch patches_array_start;
    };
}
