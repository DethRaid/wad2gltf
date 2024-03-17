#pragma once

#include <nlohmann/json.hpp>

#include "mesh.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Thing, type, flags);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Sector, light_level, special_type, tag_number);

enum class Type {
    Thing,
    Line,
    Sector,
};

template <typename DataType>
struct BaseExtra {
    Type type;
    DataType data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BaseExtra, type, data);
};

using ThingExtra = BaseExtra<Thing>;
using SectorExtra = BaseExtra<Sector>;
