#include "map_reader.hpp"

#include <format>
#include <iostream>

#include "wad.hpp"

void validate_lump_name(const LumpInfo& lump, const std::string_view name)
{
    if (memcmp(lump.name, name.data(), name.size()) != 0)
    {
        throw std::runtime_error{
            std::format("Invalid lump name! Expected: {} Actual: {}", name, std::string_view{lump.name, 8})
        };
    }
}

template <typename LumpDataType>
std::span<const LumpDataType> get_lump_data(const uint8_t* data_ptr, const LumpInfo& lump)
{
    const auto* lump_data_ptr = reinterpret_cast<const LumpDataType*>(data_ptr + lump.filepos);
    return std::span{lump_data_ptr, static_cast<size_t>(lump.size) / sizeof(LumpDataType)};
}

std::vector<Face> create_mesh_from_map(const WAD& wad, std::string_view map_name)
{
    auto itr = std::ranges::find_if(wad.lump_directory, [&](const LumpInfo& lump)
    {
        return memcmp(lump.name, map_name.data(), map_name.size()) == 0;
    });
    if (itr == wad.lump_directory.end())
    {
        throw std::runtime_error{std::format("Could not find requested map {}", map_name)};
    }

    std::cout << "Map lump " << itr->name << " Offset=" << itr->filepos << " Size=" << itr->size << "\n";

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

    // Prepare nice spans so we can access the data nicely

    const auto* data_ptr = wad.raw_data.data();

    const auto linedefs = get_lump_data<LineDef>(data_ptr, *linedefs_itr);
    const auto sidedefs = get_lump_data<SideDef>(data_ptr, *sidedefs_itr);
    const auto vertexes = get_lump_data<Vertex>(data_ptr, *vertexes_itr);
    const auto sectors = get_lump_data<Sector>(data_ptr, *sectors_itr);

    /*
     * So... how to make a mesh from all this?
     * The vertexes have the xy position of each vertex. Linedefs link different vertexes together,
     * and specify the sidedef for the front face and optionally the back face. Sidedefs specify
     * the texture to use on that size, and the sector that the side of the line belongs to.
     * Sectors define the floor and ceiling height, along with the textures to use on the floor and
     * ceiling. There's some other stuff in there but I don't understand it yet so hopefully it's
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

    auto faces = std::vector<Face>{};
    faces.reserve(linedefs.size() * 2);

    // TODO: Group faces by sector?

    for (const auto& linedef : linedefs)
    {
        const auto& start_vertex = vertexes[linedef.start_vertex];
        const auto& end_vertex = vertexes[linedef.end_vertex];

        // Front face
        {
            const auto& side = sidedefs[linedef.front_sidedef];
            const auto& sector = sectors[side.sector_number];

            const auto position0 = glm::vec3{ start_vertex.x, start_vertex.y, sector.floor_height };
            const auto position1 = glm::vec3{ end_vertex.x, end_vertex.y, sector.floor_height };
            const auto position2 = glm::vec3{ start_vertex.x, start_vertex.y, sector.ceiling_height };
            const auto position3 = glm::vec3{ end_vertex.x, end_vertex.y, sector.ceiling_height };

            const auto v0 = glm::normalize(position1 - position0);
            const auto v1 = glm::normalize(position2 - position1);
            const auto normal = glm::cross(v0, v1);

            auto& face = faces.emplace_back();
            face.vertices = std::array{
                FaceVertex{
                    .position = position0,
                    .normal = normal,
                    .texcoord = {}
                },
                FaceVertex{
                    .position = position1,
                    .normal = normal,
                    .texcoord = {}
                },
                FaceVertex{
                    .position = position2,
                    .normal = normal,
                    .texcoord = {}
                },
                FaceVertex{
                    .position = position3,
                    .normal = normal,
                    .texcoord = {}
                }
            };
        }

        // Optional back face
        if(linedef.back_sidedef > 0)
        {
            const auto& sidedef = sidedefs[linedef.back_sidedef];
            const auto& sector = sectors[sidedef.sector_number];

            const auto position0 = glm::vec3{ end_vertex.x, end_vertex.y, sector.floor_height };
            const auto position1 = glm::vec3{ start_vertex.x, start_vertex.y, sector.floor_height };
            const auto position2 = glm::vec3{ end_vertex.x, end_vertex.y, sector.ceiling_height };
            const auto position3 = glm::vec3{ start_vertex.x, start_vertex.y, sector.ceiling_height };

            const auto v0 = glm::normalize(position1 - position0);
            const auto v1 = glm::normalize(position2 - position1);
            const auto normal = glm::cross(v0, v1);

            auto& face = faces.emplace_back();
            face.vertices = std::array{
                FaceVertex{
                    .position = position0,
                    .normal = normal,
                    .texcoord = {}
                },
                FaceVertex{
                    .position = position1,
                    .normal = normal,
                    .texcoord = {}
                },
                FaceVertex{
                    .position = position2,
                    .normal = normal,
                    .texcoord = {}
                },
                FaceVertex{
                    .position = position3,
                    .normal = normal,
                    .texcoord = {}
                }
            };
        }
    }

    return faces;
}
