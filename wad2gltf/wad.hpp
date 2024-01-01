#pragma once

#include <cstdint>
#include <cstring>
#include <format>
#include <vector>
#include <span>
#include <stdexcept>
#include <unordered_map>

namespace wad {
    struct Name {
        char val[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        bool operator==(std::string_view str) const;

        bool operator==(const Name& other) const;

        std::string to_string() const;
    };

    inline bool Name::operator==(const std::string_view str) const {
        return memcmp(val, str.data(), std::min(str.size(), 8ull)) == 0;
    }

    inline bool Name::operator==(const Name& other) const {
        for(auto i = 0; i < 8; i++) {
            if (val[i] != other.val[i]) {
                return false;
            }

            if(val[i] == other.val[i] && val[i] == '\0') {
                // We've iterated to the end of the string, and we've reached the null terminator. The names are equal
                return true;
            }
        }

        // We reached the end of the string without finding different characters. These are both eight-character names
        return true;
    }

    inline std::string Name::to_string() const {
        const auto length = strnlen_s(val, 8);
        return std::string{ val, length };
    }

    struct Header {
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

    struct LumpInfo {
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
        Name name;
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
    struct WAD {
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

        template<typename NameType>
        auto find_lump(const NameType& lump_name) const {
            const auto itr = std::ranges::find_if(
                lump_directory, [&](const LumpInfo& lump) {
                    return lump.name == lump_name;
                }
            );
            if (itr == lump_directory.end()) {
                throw std::runtime_error{std::format("Could not find requested map {}", lump_name)};
            }

            return itr;
        }

        template <typename LumpDataType>
        std::span<const LumpDataType> get_lump_data(const LumpInfo& lump) const {
            const auto* lump_data_ptr = reinterpret_cast<const LumpDataType*>(raw_data.data() + lump.filepos);
            return std::span{lump_data_ptr, static_cast<size_t>(lump.size) / sizeof(LumpDataType)};
        }
    };

    struct Vertex {
        int16_t x = 0;
        int16_t y = 0;
    };

    struct LineDef {
        uint16_t start_vertex = 0;
        uint16_t end_vertex = 0;
        int16_t flags = 0;
        int16_t special_type = 0;
        int16_t sector_tag = 0;
        int16_t front_sidedef = 0;
        int16_t back_sidedef = 0;
    };

    struct SideDef {
        int16_t x_offset = 0;
        int16_t y_offset = 0;
        Name upper_texture_name;
        Name lower_texture_name;
        Name middle_texture_name;
        int16_t sector_number = 0;
    };

    struct Sector {
        int16_t floor_height = 0;
        int16_t ceiling_height = 0;
        Name floor_texture;
        Name ceiling_texture;
        int16_t light_level = 0;
        int16_t special_type = 0;
        int16_t tag_number = 0;
    };

    struct Texture1 {
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
    struct MapPatch {
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

    struct MapTexture {
        /**
         * Name of this map texture
         */
        Name name;

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

    struct PatchHeader {
        /**
         * Width of the patch
         */
        uint16_t width = 0;
        /**
         * Height of the patch
         */
        uint16_t height = 0;
        /**
         * Horizontal offset of the patch, to the left of the origin
         *
         * MapPatch has an offset, why is there one here too?
         */
        int16_t offset_x = 0;
        /**
         * Vertical offset of the patch, below the origin
         *
         * MapPatch has an offset... why is there one here too?
         */
        int16_t offset_y = 0;

        /**
         * Start of the array of offsets to the column data. There are `width` columns. The offsets are relative to the
         * start of the patch header
         */
        uint32_t column_offsets_start = 0;
    };

    struct Post {
        uint8_t row = 0;
        uint8_t height = 0;
        uint8_t unused = 0;
        uint8_t data_start;
        // There's another byte of padding at the end, allegedly to prevent overflow (?)
    };

    struct PatchColumn {
        // There's an unknown number of posts in the column. We read them until we get a post with a row of 0xFF
        Post post_start;
    };
}

template <>
struct std::formatter<wad::Name> : std::formatter<std::string> {
    auto format(const wad::Name& name, format_context& ctx) const {
        const auto length = strnlen_s(name.val, 8);
        const auto view = std::string_view{ name.val, length };
        return formatter<string>::format(std::format("{}", view), ctx);
    }
};

template <>
struct std::hash<wad::Name> {
    std::size_t operator()(const wad::Name& name) const noexcept {
        return std::hash<std::string>{}(name.to_string());
    }
};
