#include "map_reader.hpp"

#include <format>
#include <iostream>

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

    auto emit_face = [&](
        const wad::Vertex& v0, const wad::Vertex& v1, const int16_t bottom, const int16_t top,
        const wad::Name& texture_name, const float pegged_height, std::vector<Face>& destination
    ) {
        auto face = create_face(v0, v1, bottom, top);
        face.texture_index = map.get_texture_index(texture_name, wad);

        const auto line_length = glm::distance(
            glm::vec2{v0.x, v0.y}, glm::vec2{v1.x, v1.y}
        );

        face.vertices[0].texcoord = glm::vec2{0, pegged_height - face.vertices[0].position.z};
        face.vertices[1].texcoord = glm::vec2{line_length, pegged_height - face.vertices[1].position.z};
        face.vertices[2].texcoord = glm::vec2{0, pegged_height - face.vertices[2].position.z};
        face.vertices[3].texcoord = glm::vec2{line_length, pegged_height - face.vertices[3].position.z};

        destination.emplace_back(face);
    };

    const auto is_front_lower = front_sector.floor_height <= back_sector.floor_height;
    const bool is_front_side_higher = front_sector.ceiling_height >= back_sector.ceiling_height;

    const auto higher_ceiling_height = is_front_side_higher ? front_sector.ceiling_height : back_sector.ceiling_height;
    const auto higher_floor_height = is_front_lower ? back_sector.floor_height : front_sector.floor_height;

    const auto line_length = glm::distance(
        glm::vec2{start_vertex.x, start_vertex.y}, glm::vec2{end_vertex.x, end_vertex.y}
    );

    // Emit the floor-to-middle face
    if (front_sector.floor_height != back_sector.floor_height) {
        const auto pegged_height = linedef.flags & wad::LineDef::LowerTextureUnpegged
                                       ? higher_ceiling_height
                                       : higher_floor_height;

        if (is_front_lower) {
            emit_face(
                start_vertex, end_vertex, front_sector.floor_height, back_sector.floor_height,
                front_sidedef.lower_texture_name, pegged_height, boundary.front_sidedef_faces
            );
        } else {
            emit_face(
                end_vertex, start_vertex, back_sector.floor_height, front_sector.floor_height,
                back_sidedef.lower_texture_name, pegged_height, boundary.back_sidedef_faces
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
                pegged_height, boundary.front_sidedef_faces
            );
        }

        if (back_sidedef.middle_texture_name.is_valid()) {
            emit_face(
                end_vertex, start_vertex, floor_height, ceiling_height, back_sidedef.middle_texture_name,
                pegged_height, boundary.back_sidedef_faces
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
                front_sidedef.upper_texture_name, pegged_height, boundary.front_sidedef_faces
            );
        } else if (back_sidedef.upper_texture_name.is_valid()) {
            emit_face(
                end_vertex, start_vertex, front_sector.ceiling_height, back_sector.ceiling_height,
                back_sidedef.upper_texture_name, pegged_height, boundary.back_sidedef_faces
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

    auto& faces = map.sectors[sidedef.sector_number].faces;
    auto& face = faces.emplace_back();
    face = create_face(start_vertex, end_vertex, sector.floor_height, sector.ceiling_height);

    const auto vertical_uv_distance = (face.vertices[2].position.z - face.vertices[0].position.z) / 64.f;

    const auto line_length = glm::distance(
        glm::vec2{start_vertex.x, start_vertex.y}, glm::vec2{end_vertex.x, end_vertex.y}
    );
    const auto line_length_uv = line_length / 64.f;

    // If the linedef has the "lower unpegged" flag, the bottom of the texture should be at the floor
    if (flags & wad::LineDef::LowerTextureUnpegged) {
        // v0 is the bottom start, v1 is the bottom end, v2 is the top start, v3 is the top end
        // v0 and v1 should have texcoords with y = 0

        face.vertices[0].texcoord = glm::vec2{0, 1};
        face.vertices[1].texcoord = glm::vec2{line_length_uv, 1};
        face.vertices[2].texcoord = glm::vec2{0, 1 - vertical_uv_distance};
        face.vertices[3].texcoord = glm::vec2{line_length_uv, 1 - vertical_uv_distance};
    } else {
        face.vertices[0].texcoord = glm::vec2{0, vertical_uv_distance};
        face.vertices[1].texcoord = glm::vec2{line_length_uv, vertical_uv_distance};
        face.vertices[2].texcoord = glm::vec2{0, 0};
        face.vertices[3].texcoord = glm::vec2{line_length_uv, 0};
    }

    face.texture_index = map.get_texture_index(sidedef.middle_texture_name, wad);
}

Map create_mesh_from_map(const wad::WAD& wad, std::string_view map_name) {
    auto itr = wad.find_lump(map_name);

    std::print(std::cout, "Loaded map lump {}\n", itr->name);

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
    map.sectors.resize(sectors.size());
    map.textures.reserve(sectors.size() * 2);

    for (const auto& linedef : linedefs) {
        const auto& start_vertex = vertexes[linedef.start_vertex];
        const auto& end_vertex = vertexes[linedef.end_vertex];

        const auto line_length = glm::distance(
            glm::vec2{start_vertex.x, start_vertex.y}, glm::vec2{end_vertex.x, end_vertex.y}
        );
        
        const auto& front_sidedef = sidedefs[linedef.front_sidedef];

        TODO: Apply the map texture's offset

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
    }

    return map;
}
