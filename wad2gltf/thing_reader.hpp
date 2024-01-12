#pragma once
#include <cstdint>
#include <vector>

#include "wad_name.hpp"

namespace ThingFlags {
    enum Class : uint8_t {
        ArtifactItem = 1 << 0,
        Pickup = 1 << 2,
        Weapon = 1 << 3,
        Monster = 1 << 4,
        Obstacle = 1 << 5,
        Shootable = 1 << 6,
        HangsFromCeiling = 1 << 7,
    };
}

struct ThingDef {
    uint16_t id;
    uint8_t radius;
    uint8_t height;
    wad::Name sprite;
    wad::Name sequence;
    uint8_t class_flags;
};

const ThingDef& get_thing(uint16_t thing_id);
