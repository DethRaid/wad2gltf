#include "sector.hpp"

bool is_polygon_clockwise(const std::vector<SectorVertex>& polygon) {
    int32_t area = 0;
    for (auto i = 0u; i < polygon.size(); i++) {
        auto j = (i + 1) % polygon.size();

        const auto& v0 = polygon[i];
        const auto& v1 = polygon[j];

        area += (int32_t)v0[0] * (int32_t)v1[1] - (int32_t)v1[0] * (int32_t)v0[1];
    }

    return area > 0;
}

// Adapted from https://paulbourke.net/geometry/polygonmesh/#insidepoly
// NOTE: vertex positions get promoted from int16 to int32 to prevent overflow
bool is_point_in_polygon(const SectorVertex& p,const std::vector<SectorVertex>& polygon) {
    bool result = false;
    const int32_t p_x = p[0];
    const int32_t p_y = p[1];
    for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        const int32_t i_x = polygon[i][0];
        const int32_t i_y = polygon[i][1];
        const int32_t j_x = polygon[j][0];
        const int32_t j_y = polygon[j][1];
        if ((((p_y >= i_y) && (p_y < j_y)) ||
            ((p_y >= j_y) && (p_y < i_y))) &&
            (p_x < ((j_x - i_x) * (p_y - i_y) / (j_y - i_y) + i_x))
            ) {
            result = !result;
        }
    }

    return result;
}

bool is_polygon_in_polygon(const std::vector<SectorVertex>& candidate_hole,const std::vector<SectorVertex>& outer_polygon) {
    for (const auto& p : candidate_hole) {
        if (is_point_in_polygon(p, outer_polygon)) {
            return true;
        }
    }

    return false;
}
