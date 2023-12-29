#pragma once

#include <cstdint>

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
 */
struct WAD
{
    // Header
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

    std::vector<LumpInfo> lump_directory;
};
