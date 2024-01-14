#include "thing_reader.hpp"

#include <format>
#include <iostream>
#include <stb_image.h>

#include "map_reader.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"

/**
 * \brief ALL the things!
 *
 * Contains Things from DOOM Shareware, DOOM Retail, and DOOM 2
 *
 * Does not contain Things from other games
 *
 * This could be refactored into a CSV file or something, if you want to adapt this code for other games
 */
static inline std::vector all_things = {
    // Monsters
    ThingDef{
        .id = 68, .radius = 64, .height = 64, .sprite = {"BSPI"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 66, .radius = 20, .height = 56, .sprite = {"VILE"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 3003, .radius = 24, .height = 64, .sprite = {"BOSS"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 3005, .radius = 31, .height = 54, .sprite = {"HEAD"}, .sequence = {"A+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 72, .radius = 16, .height = 72, .sprite = {"KEEN"}, .sequence = {"A+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 16, .radius = 40, .height = 110, .sprite = {"CYBR"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 3002, .radius = 30, .height = 54, .sprite = {"SARG"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 65, .radius = 20, .height = 56, .sprite = {"CPOS"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 69, .radius = 24, .height = 64, .sprite = {"BOS2"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 3001, .radius = 20, .height = 56, .sprite = {"TROO"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 3006, .radius = 16, .height = 56, .sprite = {"SKUL"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 67, .radius = 48, .height = 64, .sprite = {"FATT"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 71, .radius = 31, .height = 56, .sprite = {"PAIN"}, .sequence = {"A+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 66, .radius = 20, .height = 56, .sprite = {"SKEL"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 9, .radius = 20, .height = 56, .sprite = {"SPOS"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 58, .radius = 30, .height = 56, .sprite = {"SARG"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 7, .radius = 128, .height = 100, .sprite = {"SPID"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 84, .radius = 20, .height = 56, .sprite = {"SSWV"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 3004, .radius = 20, .height = 56, .sprite = {"POSS"}, .sequence = {"AB+"},
        .class_flags = ThingFlags::Monster | ThingFlags::Obstacle | ThingFlags::Shootable
    },

    // Weapons
    ThingDef{
        .id = 2006, .radius = 20, .height = 16, .sprite = {"BFUG"}, .sequence = {"A"},
        .class_flags = ThingFlags::Weapon | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2002, .radius = 20, .height = 16, .sprite = {"MGUN"}, .sequence = {"A"},
        .class_flags = ThingFlags::Weapon | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2005, .radius = 20, .height = 16, .sprite = {"CSAW"}, .sequence = {"A"},
        .class_flags = ThingFlags::Weapon | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2004, .radius = 20, .height = 16, .sprite = {"PLAS"}, .sequence = {"A"},
        .class_flags = ThingFlags::Weapon | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2003, .radius = 20, .height = 16, .sprite = {"LAUN"}, .sequence = {"A"},
        .class_flags = ThingFlags::Weapon | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2001, .radius = 20, .height = 16, .sprite = {"SHOT"}, .sequence = {"A"},
        .class_flags = ThingFlags::Weapon | ThingFlags::Pickup
    },
    ThingDef{
        .id = 82, .radius = 20, .height = 16, .sprite = {"SGN2"}, .sequence = {"A"},
        .class_flags = ThingFlags::Weapon | ThingFlags::Pickup
    },

    // Ammo
    ThingDef{
        .id = 2008, .radius = 20, .height = 16, .sprite = {"SHEL"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 2048, .radius = 20, .height = 16, .sprite = {"AMMO"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 2046, .radius = 20, .height = 16, .sprite = {"BROK"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 2049, .radius = 20, .height = 16, .sprite = {"SBOX"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 2007, .radius = 20, .height = 16, .sprite = {"CLIP"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 2047, .radius = 20, .height = 16, .sprite = {"CELL"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 17, .radius = 20, .height = 16, .sprite = {"CELP"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 2010, .radius = 20, .height = 16, .sprite = {"ROCK"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },

    // Artifacts
    ThingDef{
        .id = 2015, .radius = 20, .height = 16, .sprite = {"BON2"}, .sequence = {"ABCDCB"},
        .class_flags = ThingFlags::ArtifactItem | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2023, .radius = 20, .height = 16, .sprite = {"PSTR"}, .sequence = {"A"},
        .class_flags = ThingFlags::ArtifactItem | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2026, .radius = 20, .height = 16, .sprite = {"PMAP"}, .sequence = {"ABCDCB"},
        .class_flags = ThingFlags::ArtifactItem | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2014, .radius = 20, .height = 16, .sprite = {"BON1"}, .sequence = {"ABCDCB"},
        .class_flags = ThingFlags::ArtifactItem | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2022, .radius = 20, .height = 16, .sprite = {"PINV"}, .sequence = {"ABCD"},
        .class_flags = ThingFlags::ArtifactItem | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2045, .radius = 20, .height = 16, .sprite = {"PVIS"}, .sequence = {"AB"},
        .class_flags = ThingFlags::ArtifactItem | ThingFlags::Pickup
    },
    ThingDef{
        .id = 83, .radius = 20, .height = 16, .sprite = {"MEGA"}, .sequence = {"ABCD"},
        .class_flags = ThingFlags::ArtifactItem | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2024, .radius = 20, .height = 16, .sprite = {"PINS"}, .sequence = {"ABCD"},
        .class_flags = ThingFlags::ArtifactItem | ThingFlags::Pickup
    },
    ThingDef{
        .id = 2013, .radius = 20, .height = 16, .sprite = {"SOUL"}, .sequence = {"ABCDCB"},
        .class_flags = ThingFlags::ArtifactItem | ThingFlags::Pickup
    },

    // Powerups (NOT artifacts, these are very different)
    ThingDef{
        .id = 2018, .radius = 20, .height = 16, .sprite = {"ARM1"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 8, .radius = 20, .height = 16, .sprite = {"BPAK"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 2012, .radius = 20, .height = 16, .sprite = {"MEDI"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 2019, .radius = 20, .height = 16, .sprite = {"ARM2"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 2025, .radius = 20, .height = 16, .sprite = {"SUIT"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 2011, .radius = 20, .height = 16, .sprite = {"STIM"}, .sequence = {"A"},
        .class_flags = ThingFlags::Pickup
    },

    // Keys
    ThingDef{
        .id = 5, .radius = 20, .height = 16, .sprite = {"BKEY"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 20, .radius = 20, .height = 16, .sprite = {"BSKU"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 13, .radius = 20, .height = 16, .sprite = {"RKEY"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 38, .radius = 20, .height = 16, .sprite = {"RSKU"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 6, .radius = 20, .height = 16, .sprite = {"YKEY"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Pickup
    },
    ThingDef{
        .id = 39, .radius = 20, .height = 16, .sprite = {"YSKU"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Pickup
    },

    // Obstacles
    ThingDef{
        .id = 47, .radius = 16, .height = 16, .sprite = {"SMIT"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 70, .radius = 16, .height = 16, .sprite = {"FCAN"}, .sequence = {"ABC"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 43, .radius = 16, .height = 16, .sprite = {"TRE1"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 35, .radius = 16, .height = 16, .sprite = {"CBRA"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 41, .radius = 16, .height = 16, .sprite = {"CEYE"}, .sequence = {"ABCB"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 2035, .radius = 16, .height = 16, .sprite = {"BAR1"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 28, .radius = 16, .height = 16, .sprite = {"POL2"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 42, .radius = 16, .height = 16, .sprite = {"FSKU"}, .sequence = {"ABC"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 2028, .radius = 16, .height = 16, .sprite = {"COLU"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 53, .radius = 16, .height = 16, .sprite = {"GOR5"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 52, .radius = 16, .height = 16, .sprite = {"GOR4"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 78, .radius = 16, .height = 16, .sprite = {"HDB6"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 75, .radius = 16, .height = 16, .sprite = {"HDB3"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 77, .radius = 16, .height = 16, .sprite = {"HDB5"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 76, .radius = 16, .height = 16, .sprite = {"HDB4"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 50, .radius = 16, .height = 16, .sprite = {"GOR2"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 74, .radius = 16, .height = 16, .sprite = {"HDB2"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 73, .radius = 16, .height = 16, .sprite = {"HDB1"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 51, .radius = 16, .height = 16, .sprite = {"GOR3"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 49, .radius = 16, .height = 16, .sprite = {"GOR1"}, .sequence = {"ABCB"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::HangsFromCeiling
    },
    ThingDef{
        .id = 25, .radius = 16, .height = 16, .sprite = {"POL1"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 54, .radius = 16, .height = 16, .sprite = {"TRE2"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 29, .radius = 16, .height = 16, .sprite = {"POL3"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 55, .radius = 16, .height = 16, .sprite = {"SMBT"}, .sequence = {"ABCD"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 56, .radius = 16, .height = 16, .sprite = {"SMGT"}, .sequence = {"ABCD"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 31, .radius = 16, .height = 16, .sprite = {"COL2"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 36, .radius = 16, .height = 16, .sprite = {"COL5"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 57, .radius = 16, .height = 16, .sprite = {"SMRT"}, .sequence = {"ABCD"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 33, .radius = 16, .height = 16, .sprite = {"COL4"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 37, .radius = 16, .height = 16, .sprite = {"COL6"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 86, .radius = 16, .height = 16, .sprite = {"TLP2"}, .sequence = {"ABCD"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 27, .radius = 16, .height = 16, .sprite = {"POL4"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 44, .radius = 16, .height = 16, .sprite = {"TBLU"}, .sequence = {"ABCD"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 45, .radius = 16, .height = 16, .sprite = {"TGRN"}, .sequence = {"ABCD"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 30, .radius = 16, .height = 16, .sprite = {"COL1"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 46, .radius = 16, .height = 16, .sprite = {"TRED"}, .sequence = {"ABCD"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 32, .radius = 16, .height = 16, .sprite = {"COL3"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 48, .radius = 16, .height = 16, .sprite = {"ELEC"}, .sequence = {"A"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 85, .radius = 16, .height = 16, .sprite = {"TLMP"}, .sequence = {"ABCD"},
        .class_flags = ThingFlags::Obstacle
    },
    ThingDef{
        .id = 26, .radius = 16, .height = 16, .sprite = {"POL6"}, .sequence = {"AB"},
        .class_flags = ThingFlags::Obstacle
    },

    // Decorations
    ThingDef{
        .id = 10, .radius = 20, .height = 16, .sprite = {"PLAY"}, .sequence = {"W"},
        .class_flags = 0
    },
    ThingDef{
        .id = 12, .radius = 20, .height = 16, .sprite = {"PLAY"}, .sequence = {"W"},
        .class_flags = 0
    },
    ThingDef{
        .id = 34, .radius = 20, .height = 16, .sprite = {"CAND"}, .sequence = {"A"},
        .class_flags = 0
    },
    ThingDef{
        .id = 22, .radius = 20, .height = 16, .sprite = {"HEAD"}, .sequence = {"L"},
        .class_flags = 0
    },
    ThingDef{
        .id = 21, .radius = 20, .height = 16, .sprite = {"SARG"}, .sequence = {"N"},
        .class_flags = 0
    },
    ThingDef{
        .id = 18, .radius = 20, .height = 16, .sprite = {"POSS"}, .sequence = {"L"},
        .class_flags = 0
    },
    ThingDef{
        .id = 19, .radius = 20, .height = 16, .sprite = {"SPOS"}, .sequence = {"L"},
        .class_flags = 0
    },
    ThingDef{
        .id = 20, .radius = 20, .height = 16, .sprite = {"TROO"}, .sequence = {"M"},
        .class_flags = 0
    },
    ThingDef{
        .id = 23, .radius = 20, .height = 16, .sprite = {"SKUL"}, .sequence = {"K"},
        .class_flags = 0
    },
    ThingDef{
        .id = 15, .radius = 20, .height = 16, .sprite = {"PLAY"}, .sequence = {"N"},
        .class_flags = 0
    },
    ThingDef{
        .id = 62, .radius = 20, .height = 52, .sprite = {"GOR5"}, .sequence = {"A"},
        .class_flags = 0
    },
    ThingDef{
        .id = 60, .radius = 20, .height = 68, .sprite = {"GOR4"}, .sequence = {"A"},
        .class_flags = 0
    },
    ThingDef{
        .id = 59, .radius = 20, .height = 84, .sprite = {"GOR2"}, .sequence = {"A"},
        .class_flags = 0
    },
    ThingDef{
        .id = 61, .radius = 20, .height = 52, .sprite = {"GOR3"}, .sequence = {"A"},
        .class_flags = 0
    },
    ThingDef{
        .id = 63, .radius = 20, .height = 68, .sprite = {"GOR1"}, .sequence = {"ABCB"},
        .class_flags = 0
    },
    ThingDef{
        .id = 79, .radius = 20, .height = 16, .sprite = {"POB1"}, .sequence = {"A"},
        .class_flags = 0
    },
    ThingDef{
        .id = 80, .radius = 20, .height = 16, .sprite = {"POB2"}, .sequence = {"A"},
        .class_flags = 0
    },
    ThingDef{
        .id = 24, .radius = 20, .height = 16, .sprite = {"POL5"}, .sequence = {"A"},
        .class_flags = 0
    },
    ThingDef{
        .id = 81, .radius = 20, .height = 16, .sprite = {"BRS1"}, .sequence = {"A"},
        .class_flags = 0
    },

    // Others
    ThingDef{
        .id = 11, .radius = 20, .height = 56, .sprite = {}, .sequence = {"-"},
        .class_flags = 0
    },
    ThingDef{
        .id = 89, .radius = 20, .height = 32, .sprite = {}, .sequence = {"-"},
        .class_flags = 0
    },
    ThingDef{
        .id = 1, .radius = 16, .height = 56, .sprite = {"PLAY"}, .sequence = {"A+"},
        .class_flags = 0
    },
    ThingDef{
        .id = 2, .radius = 16, .height = 56, .sprite = {"PLAY"}, .sequence = {"A+"},
        .class_flags = 0
    },
    ThingDef{
        .id = 3, .radius = 16, .height = 56, .sprite = {"PLAY"}, .sequence = {"A+"},
        .class_flags = 0
    },
    ThingDef{
        .id = 4, .radius = 16, .height = 56, .sprite = {"PLAY"}, .sequence = {"A+"},
        .class_flags = 0
    },
    ThingDef{
        .id = 88, .radius = 16, .height = 16, .sprite = {"BBRN"}, .sequence = {"A+"},
        .class_flags = ThingFlags::Obstacle | ThingFlags::Shootable
    },
    ThingDef{
        .id = 87, .radius = 20, .height = 32, .sprite = {}, .sequence = {"-"},
        .class_flags = 0
    },
    ThingDef{
        .id = 14, .radius = 20, .height = 16, .sprite = {}, .sequence = {"-"},
        .class_flags = 0
    },
};

bool ThingDef::is_spriteless() const {
    // "others" don't have sprites that follow the same rules as other THINGS, so ignore them for now
    return id == 11 || id == 89 || id == 1 || id == 2 || id == 3 || id == 4 || id == 88 || id == 87 || id == 14;
}

void load_things_into_map(const wad::WAD& wad, const MapExtractionOptions& options, Map& map) {
    if (!options.export_things) {
        return;
    }

    auto itr = wad.find_lump(options.map_name);

    const auto map_lump_itr = itr;
    ++itr;
    const auto things_itr = itr;

    // There's some other lumps that don't seem important for this program

    const auto wad_things = wad.get_lump_data<wad::Thing>(*things_itr);

    map.things.reserve(wad_things.size());

    // Copy the Things
    for (const auto& thing : wad_things) {
        const auto& thing_def = get_thing(thing.type);

        if (thing_def.is_spriteless()) {
            // Skip spriteless things because coding is h ard
            continue;
        }

        auto sector_floor = float{0};
        for (const auto& sector : map.sectors) {
            for (const auto& polygon : sector.exterior_loops) {
                if (is_point_in_polygon({ thing.x, thing.y }, polygon)) {
                    sector_floor = sector.floor.vertices[0].z;
                    break;
                }
            }
        }

        auto thing_sprite = load_sprite_from_wad(thing_def.sprite, wad);

        auto face = Face{
            .vertices = {
                Vertex{
                    .position = {0, -thing_sprite.size.x / 2.f, thing_sprite.size.y},
                    .texcoord = {1, 0}
                },
                Vertex{
                    .position = {0, thing_sprite.size.x / 2.f, thing_sprite.size.y},
                    .texcoord = {0, 0}
                },
                Vertex{
                    .position = {0, -thing_sprite.size.x / 2.f, 0.f},
                    .texcoord = {1, 1}
                },
                Vertex{
                    .position = {0, thing_sprite.size.x / 2.f, 0.f},
                    .texcoord = {0, 1}
                },
            },
            .normal = glm::vec3{-1, 0, 0},
            .texture_index = static_cast<uint32_t>(map.textures.size())
        };
        map.textures.emplace_back(std::move(thing_sprite));

        // DOOM rotation: https://doomwiki.org/wiki/Angle

        const auto radians_angle = glm::radians(static_cast<float>(thing.facing_angle));

        map.things.emplace_back(
            glm::vec3{thing.x, thing.y, sector_floor}, radians_angle, face,
            thing.type, thing.flags
        );
    }

    std::cout << std::format("Loaded THINGS from map {}\n", options.map_name);
}

const ThingDef& get_thing(uint16_t thing_id) {
    const auto itr = std::find_if(
        all_things.begin(), all_things.end(), [=](const ThingDef& thing) { return thing.id == thing_id; }
    );
    if (itr == all_things.end()) {
        throw std::runtime_error{std::format("No thing with ID {}", thing_id)};
    }

    return *itr;
}
