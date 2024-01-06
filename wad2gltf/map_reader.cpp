#include "map_reader.hpp"

#include <algorithm>
#include <format>
#include <iostream>
#include <numeric>
#include <set>

#include <mapbox/earcut.hpp>

#include "wad.hpp"

struct SectorBoundaryFaces {
    std::vector<Face> back_sidedef_faces;
    std::vector<Face> front_sidedef_faces;
};

void validate_lump_name(const wad::LumpInfo& lump, const std::string_view name) {
    if (lump.name != name) {
        throw std::runtime_error{
            std::format("Invalid lump name! Expected: {} Actual: {}", name, std::string_view{lump.name.val, 8})
        };
    }
}

Face create_face(
    const wad::Vertex& v0, const wad::Vertex v1, const int32_t bottom_height, const int32_t top_height
) {
    auto face = Face{};

    const auto position0 = glm::vec3{v0.x, v0.y, bottom_height};
    const auto position1 = glm::vec3{v1.x, v1.y, bottom_height};
    const auto position2 = glm::vec3{v0.x, v0.y, top_height};
    const auto position3 = glm::vec3{v1.x, v1.y, top_height};

    const auto e0 = glm::normalize(position1 - position0);
    const auto e1 = glm::normalize(position2 - position1);
    face.normal = normalize(glm::cross(e0, e1));

    face.vertices = std::array{
        Vertex{
            .position = position0,
            .texcoord = {}
        },
        Vertex{
            .position = position1,
            .texcoord = {}
        },
        Vertex{
            .position = position2,
            .texcoord = {}
        },
        Vertex{
            .position = position3,
            .texcoord = {}
        }
    };

    return face;
}

void emit_face(
    const wad::Vertex& v0, const wad::Vertex& v1, const int16_t bottom, const int16_t top,
    const wad::Name& texture_name, const glm::i16vec2& texture_offset, const float pegged_height,
    std::vector<Face>& destination, const wad::WAD& wad,
    Map& map
) {
    if(texture_name.is_none()) {
        // The Unofficial Doom Specs state that "-" means not rendered, so don't generate the face
        return;
    }

    auto face = create_face(v0, v1, bottom, top);
    face.texture_index = map.get_texture_index(texture_name, wad);

    const auto line_length = glm::distance(
        glm::vec2{v0.x, v0.y}, glm::vec2{v1.x, v1.y}
    );

    face.vertices[0].texcoord = glm::vec2{0, pegged_height - face.vertices[0].position.z};
    face.vertices[1].texcoord = glm::vec2{line_length, pegged_height - face.vertices[1].position.z};
    face.vertices[2].texcoord = glm::vec2{0, pegged_height - face.vertices[2].position.z};
    face.vertices[3].texcoord = glm::vec2{line_length, pegged_height - face.vertices[3].position.z};

    const auto& texture = map.textures[face.texture_index];
    // Apply offset and scale from pixels -> UV
    for (auto& vertex : face.vertices) {
        vertex.texcoord += texture_offset;
        vertex.texcoord /= glm::vec2{texture.size};
    }

    destination.emplace_back(face);
}

SectorBoundaryFaces generate_faces_for_sector_boundary(
    const wad::LineDef& linedef, const wad::Vertex& start_vertex, const wad::Vertex& end_vertex,
    const std::span<const wad::SideDef> sidedefs, const std::span<const wad::Sector> sectors, const wad::WAD& wad,
    Map& map
) {
    auto boundary = SectorBoundaryFaces{};

    const auto& front_sidedef = sidedefs[linedef.front_sidedef];
    const auto& back_sidedef = sidedefs[linedef.back_sidedef];
    const auto& front_sector = sectors[front_sidedef.sector_number];
    const auto& back_sector = sectors[back_sidedef.sector_number];

    const auto is_front_lower = front_sector.floor_height <= back_sector.floor_height;
    const bool is_front_side_higher = front_sector.ceiling_height >= back_sector.ceiling_height;

    const auto higher_ceiling_height = is_front_side_higher ? front_sector.ceiling_height : back_sector.ceiling_height;
    const auto higher_floor_height = is_front_lower ? back_sector.floor_height : front_sector.floor_height;

    // Emit the floor-to-middle face
    if (front_sector.floor_height != back_sector.floor_height) {
        const auto pegged_height = linedef.flags & wad::LineDef::LowerTextureUnpegged
                                       ? higher_ceiling_height
                                       : higher_floor_height;

        if (is_front_lower) {
            emit_face(
                start_vertex, end_vertex, front_sector.floor_height, back_sector.floor_height,
                front_sidedef.lower_texture_name, glm::i16vec2{front_sidedef.x_offset, front_sidedef.y_offset},
                pegged_height, boundary.front_sidedef_faces, wad, map
            );
        } else {
            emit_face(
                end_vertex, start_vertex, back_sector.floor_height, front_sector.floor_height,
                back_sidedef.lower_texture_name, glm::i16vec2{back_sidedef.x_offset, back_sidedef.y_offset},
                pegged_height, boundary.back_sidedef_faces, wad, map
            );
        }
    }

    // Emit any center faces
    {
        const auto floor_height = is_front_lower ? back_sector.floor_height : front_sector.floor_height;
        const auto ceiling_height = is_front_side_higher ? back_sector.ceiling_height : front_sector.ceiling_height;
        const auto pegged_height = linedef.flags & wad::LineDef::LowerTextureUnpegged ? floor_height : ceiling_height;

        if (front_sidedef.middle_texture_name.is_valid()) {
            emit_face(
                start_vertex, end_vertex, floor_height, ceiling_height, front_sidedef.middle_texture_name,
                glm::i16vec2{front_sidedef.x_offset, front_sidedef.y_offset}, pegged_height,
                boundary.front_sidedef_faces, wad, map
            );
        }

        if (back_sidedef.middle_texture_name.is_valid()) {
            emit_face(
                end_vertex, start_vertex, floor_height, ceiling_height, back_sidedef.middle_texture_name,
                glm::i16vec2{back_sidedef.x_offset, back_sidedef.y_offset},
                pegged_height, boundary.back_sidedef_faces, wad, map
            );
        }
    }

    // Emit middle-to-ceiling face
    if (front_sector.ceiling_height != back_sector.ceiling_height) {
        const auto pegged_height = linedef.flags & wad::LineDef::UpperTextureUnpegged
                                       ? higher_ceiling_height
                                       : higher_floor_height;
        if (is_front_side_higher && front_sidedef.upper_texture_name.is_valid()) {
            emit_face(
                start_vertex, end_vertex, back_sector.ceiling_height, front_sector.ceiling_height,
                front_sidedef.upper_texture_name, glm::i16vec2{front_sidedef.x_offset, front_sidedef.y_offset},
                pegged_height, boundary.front_sidedef_faces, wad, map
            );
        } else if (back_sidedef.upper_texture_name.is_valid()) {
            emit_face(
                end_vertex, start_vertex, front_sector.ceiling_height, back_sector.ceiling_height,
                back_sidedef.upper_texture_name, glm::i16vec2{back_sidedef.x_offset, back_sidedef.y_offset},
                pegged_height, boundary.back_sidedef_faces, wad, map
            );
        }
    }

    return boundary;
}

void generate_one_sided_wall(
    const wad::LineDef& linedef, const wad::Vertex& start_vertex, const wad::Vertex end_vertex,
    const wad::SideDef& sidedef, const std::span<const wad::Sector> sectors, const wad::WAD& wad, Map& map
) {
    const auto flags = linedef.flags;

    const auto& sector = sectors[sidedef.sector_number];

    // There's a sector with floor and ceiling height set to 0? Not sure why, not sure why it used to work
    if (sector.ceiling_height == sector.floor_height) {
        return;
    }

    auto& faces = map.sectors[sidedef.sector_number].faces;

    const auto pegged_height = flags & wad::LineDef::LowerTextureUnpegged ? sector.floor_height : sector.ceiling_height;
    emit_face(
        start_vertex, end_vertex, sector.floor_height, sector.ceiling_height, sidedef.middle_texture_name,
        glm::i16vec2{sidedef.x_offset, sidedef.y_offset}, pegged_height, faces, wad, map
    );
}

std::vector<std::array<int16_t, 2>> extract_line_loop(
    const std::span<const wad::Vertex> vertexes, const std::vector<std::pair<uint16_t, uint16_t>>& sector_linedefs,
    std::vector<uint32_t>& remaining_lines
) {
    auto visited_lines = std::set<size_t>{};
    auto vertices_in_loop = std::vector<std::array<int16_t, 2>>{};

    auto cur_line_idx = remaining_lines[0];
    auto iterating = true;
    while (iterating) {
        visited_lines.emplace(cur_line_idx);
        std::erase(remaining_lines, cur_line_idx);
        const auto& cur_line = sector_linedefs[cur_line_idx];

        // Add our vertex to the loop
        const auto& vertex = vertexes[cur_line.first];
        vertices_in_loop.emplace_back(std::array{vertex.x, vertex.y});

        // Find the line that continues the loop
        for (auto i = 0u; i < sector_linedefs.size(); i++) {
            const auto& test_line = sector_linedefs[i];
            if (test_line.first == cur_line.second) {
                if (visited_lines.contains(i)) {
                    // We've reached the start of the loop. Return the loop
                    return vertices_in_loop;
                } else {
                    // This is a continuation of the loop, keep iterating
                    cur_line_idx = i;
                    break;
                }
            }
        }
    }

    return vertices_in_loop;
}

Map create_mesh_from_map(const wad::WAD& wad, const MapExtractionOptions& options) {
    auto itr = wad.find_lump(options.map_name);

    std::cout << std::format("Loaded map lump {}\n", itr->name);

    // DOOM wiki says these have to be in this order
    const auto map_lump_itr = itr;
    ++itr;
    const auto things_itr = itr;
    ++itr;
    const auto linedefs_itr = itr;
    ++itr;
    const auto sidedefs_itr = itr;
    ++itr;
    const auto vertexes_itr = itr;
    ++itr;
    const auto segs_itr = itr;
    ++itr;
    const auto ssectors_itr = itr;
    ++itr;
    const auto nodes_itr = itr;
    ++itr;
    const auto sectors_itr = itr;

    // There's some other optional things that don't seem important for this program

    // Validate that the lumps are what they should be
    validate_lump_name(*things_itr, "THINGS");
    validate_lump_name(*linedefs_itr, "LINEDEFS");
    validate_lump_name(*sidedefs_itr, "SIDEDEFS");
    validate_lump_name(*vertexes_itr, "VERTEXES");
    validate_lump_name(*segs_itr, "SEGS");
    validate_lump_name(*ssectors_itr, "SSECTORS");
    validate_lump_name(*nodes_itr, "NODES");
    validate_lump_name(*sectors_itr, "SECTORS");

    const auto things = wad.get_lump_data<wad::Thing>(*things_itr);
    const auto linedefs = wad.get_lump_data<wad::LineDef>(*linedefs_itr);
    const auto sidedefs = wad.get_lump_data<wad::SideDef>(*sidedefs_itr);
    const auto vertexes = wad.get_lump_data<wad::Vertex>(*vertexes_itr);
    const auto sectors = wad.get_lump_data<wad::Sector>(*sectors_itr);

    /*
     * So... how to make a mesh from all this?
     * The vertexes have the xy position of each vertex. Linedefs link different vertexes together,
     * and specify the sidedef for the front face and optionally the back face. Sidedefs specify
     * the texture to use on that size, and the sector that the side of the line belongs to.
     * Sectors define the floor and ceiling height, along with the textures to use on the floor and
     * ceiling. There's some other stuff in there, but I don't understand it yet, so hopefully it's
     * not very important
     *
     * We can iterate through all the linedefs and emit a quad for each sidedef. We'll pull the z
     * of the floor and ceiling vertices from the sector, and pull the UV coordinates from the
     * sidedef's x and y offset. This will produce a great many individual quads
     *
     * We'll make not attempt to package the data for glTF here, but return all the quads as they
     * are. Other processing steps can refine the quad organization or just splat them into glTF
     *
     * Floors and ceilings will be a bit harder. We want to find a loop of sidedefs that enclose a
     * sector. However, a sector may contain multiple sidedef loops, and the loops themselves may
     * not be closed. Then, we have to triangulate the arbitrary polygon formed by the sidedefs.
     * That'll probably be a task for when I'm back home
     */

    auto map = Map{};
    map.things.reserve(things.size());
    map.sectors.resize(sectors.size());
    map.textures.reserve(sectors.size() * 2);

    // Copy the Things
    for(const auto& thing : things) {
        map.things.emplace_back(glm::vec2{thing.x, thing.y}, static_cast<float>(thing.facing_angle), thing.type, thing.flags);
    }

    // Copy the sector info into our data structure
    for(auto i = 0u; i < sectors.size(); i++) {
        const auto& sector = sectors[i];
        auto& map_sector = map.sectors[i];
        map_sector.light_level = sector.light_level;
        map_sector.special_type = sector.special_type;
        map_sector.tag_number = sector.tag_number;
    }

    auto linedefs_per_sector = std::vector<std::vector<std::pair<uint16_t, uint16_t>>>(sectors.size());

    for (const auto& linedef : linedefs) {
        const auto& start_vertex = vertexes[linedef.start_vertex];
        const auto& end_vertex = vertexes[linedef.end_vertex];

        const auto& front_sidedef = sidedefs[linedef.front_sidedef];

        if (linedef.flags & wad::LineDef::TwoSided) {
            // Two-sided wall

            const auto& back_sidedef = sidedefs[linedef.back_sidedef];

            const auto& [back_faces, front_faces] = generate_faces_for_sector_boundary(
                linedef, start_vertex, end_vertex, sidedefs, sectors, wad, map
            );

            auto& front_sector_faces = map.sectors[front_sidedef.sector_number].faces;
            front_sector_faces.insert(front_sector_faces.end(), front_faces.begin(), front_faces.end());

            auto& back_sector_faces = map.sectors[back_sidedef.sector_number].faces;
            back_sector_faces.insert(back_sector_faces.end(), back_faces.begin(), back_faces.end());
        } else {
            // One-sided wall
            generate_one_sided_wall(linedef, start_vertex, end_vertex, front_sidedef, sectors, wad, map);

            if (linedef.back_sidedef != -1) {
                const auto& back_sidedef = sidedefs[linedef.back_sidedef];
                // NOLINT(readability-suspicious-call-argument)
                generate_one_sided_wall(linedef, end_vertex, start_vertex, back_sidedef, sectors, wad, map);
            }
        }

        linedefs_per_sector.at(front_sidedef.sector_number).emplace_back(linedef.start_vertex, linedef.end_vertex);
        if (linedef.back_sidedef != -1) {
            const auto& back_sidedef = sidedefs[linedef.back_sidedef];
            linedefs_per_sector.at(back_sidedef.sector_number).emplace_back(linedef.end_vertex, linedef.start_vertex);
        }
    }

    // Try to split the sector into its line loops and triangulate them
    // This doesn't work for all sectors, unfortunately
    for (auto i = 0u; i < linedefs_per_sector.size(); i++) {
        const auto& sector_linedefs = linedefs_per_sector[i];

        auto remaining_lines = std::vector<uint32_t>(sector_linedefs.size());
        std::iota(remaining_lines.begin(), remaining_lines.end(), 0);

        auto polygon_line_loops = std::vector<std::vector<std::array<int16_t, 2>>>{};

        while (!remaining_lines.empty()) {
            auto loop = extract_line_loop(vertexes, sector_linedefs, remaining_lines);
            polygon_line_loops.emplace_back(loop);
        }

        // This produces zero indices for some sectors - namely, the slightly-higher blue carpet in the starting room
        // of E1M1. It seems that this fails if the provided vertices form multiple polygons
        // TODO: Split the vertices into multiple polygons
        // A line loop encloses a polygon if the sum of the angles where the lines point inside is greater than the sum
        // of the angles where the lines point outside. 
        const auto ceiling_indices = mapbox::earcut<uint16_t>(polygon_line_loops);

        // We can add the indices as-is to a ceiling flat, but we have to reverse them for a floor flat
        auto& map_sector = map.sectors[i];

        const auto& sector = sectors[i];
        map_sector.ceiling.texture_index = map.get_flat_index(sector.ceiling_texture, wad);
        map_sector.floor.texture_index = map.get_flat_index(sector.floor_texture, wad);

        // Flatten the vertices arrays
        auto vertices = std::vector<glm::vec2>{};
        vertices.reserve(polygon_line_loops.size() * polygon_line_loops[0].size());
        for (const auto& loop_vertices : polygon_line_loops) {
            for (const auto& vertex : loop_vertices) {
                vertices.emplace_back(vertex[0], vertex[1]);
            }
        }

        map_sector.ceiling.vertices.reserve(vertices.size());
        map_sector.floor.vertices.reserve(vertices.size());

        for (const auto& vertex : vertices) {
            map_sector.ceiling.vertices.emplace_back(vertex[0], vertex[1], sector.ceiling_height);
            map_sector.floor.vertices.emplace_back(vertex[0], vertex[1], sector.floor_height);
        }

        map_sector.ceiling.indices.resize(ceiling_indices.size());
        map_sector.floor.indices.resize(ceiling_indices.size());

        for (auto triangle_index = 0u; triangle_index < ceiling_indices.size(); triangle_index += 3) {
            map_sector.ceiling.indices[triangle_index] = ceiling_indices[triangle_index + 2];
            map_sector.ceiling.indices[triangle_index + 1] = ceiling_indices[triangle_index + 1];
            map_sector.ceiling.indices[triangle_index + 2] = ceiling_indices[triangle_index];

            map_sector.floor.indices[triangle_index] = ceiling_indices[triangle_index];
            map_sector.floor.indices[triangle_index + 1] = ceiling_indices[triangle_index + 1];
            map_sector.floor.indices[triangle_index + 2] = ceiling_indices[triangle_index + 2];
        }
    }

    return map;
}
