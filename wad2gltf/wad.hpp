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
 * All the pointers in this data structure refer to the raw data vector. Thus, copying this data structure is not
 * allowed. Maybe one day I'll write a good copy constructor/operator
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
