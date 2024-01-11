#pragma once

#include <array>
#include <cstdint>
#include <vector>

using SectorVertex = std::array<int16_t, 2>;

bool is_polygon_clockwise(const std::vector<SectorVertex>& polygon);

bool is_point_in_polygon(const SectorVertex& p, const std::vector<SectorVertex>& polygon);

bool is_polygon_in_polygon(
    const std::vector<SectorVertex>& candidate_hole, const std::vector<SectorVertex>& outer_polygon
);
