#pragma once

#include <cstdint>
#include <vector>
#include <span>

struct WadHeader
{
    /**
     * Identifier for the file. Must be either PWAD or IWAD
     */
    char identification[4];
    /**
     * Number of lumps in this WAD
     */
    int32_t numlumps;
    /**
     * Offset of the info table in the WAD data
     */
    int32_t infotableofs;
};

struct LumpInfo
{
    /**
     * Byte offset of the lump's data in the file
     */
    int32_t filepos;

    /**
     * Size of the lump, in bytes
     */
    int32_t size;

    /**
     * Name of the lump, hopefully null-terminated
     */
    char name[8];
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
    WadHeader* header;

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
};

struct Vertex
{
    int16_t x;
    int16_t y;
};

struct LineDef
{
    uint16_t start_vertex;
    uint16_t end_vertex;
    int16_t flags;
    int16_t special_type;
    int16_t sector_tag;
    int16_t front_sidedef;
    int16_t back_sidedef;
};

struct SideDef
{
    int16_t x_offset;
    int16_t y_offset;
    char upper_texture_name[8];
    char lower_texture_name[8];
    char middle_texture_name[8];
    int16_t sector_number;
};

struct Sector
{
    int16_t floor_height;
    int16_t ceiling_height;
    char floor_texture[8];
    char ceiling_texture[8];
    int16_t light_level;
    int16_t special_type;
    int16_t tag_number;
};
