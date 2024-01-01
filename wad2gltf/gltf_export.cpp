#include "gltf_export.hpp"

#include <format>

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/quaternion.hpp>

template <typename DataType>
void write_data_to_buffer(std::vector<uint8_t>& buffer, const std::span<const DataType> data) {
    const auto dest_offset = buffer.size();
    const auto num_bytes = data.size() * sizeof(DataType);
    buffer.resize(buffer.size() + num_bytes);
    auto* dest_ptr = buffer.data() + dest_offset;
    std::memcpy(dest_ptr, data.data(), num_bytes);
}

fastgltf::Asset export_to_gltf(const std::string_view name, const Map& map) {
    auto model = fastgltf::Asset{};
    model.defaultScene = 0;
    model.assetInfo = fastgltf::AssetInfo{.gltfVersion = "2.0", .generator = "wad2gltf"};

    auto& scene = model.scenes.emplace_back();
    scene.name = std::string{name};

    auto& sampler = model.samplers.emplace_back();
    sampler.magFilter = fastgltf::Filter::Nearest;
    sampler.minFilter = fastgltf::Filter::NearestMipMapNearest;

    // Create materials for all the map textures. We'll shrimply create a non-PBR material for each one
    for (const auto& texture : map.textures) {
        auto& gltf_material = model.materials.emplace_back();
        gltf_material.name = texture.info.name.to_string();
        // TODO: Use masked alpha for images with transparency
        gltf_material.pbrData.baseColorTexture = fastgltf::TextureInfo{
            .textureIndex = model.textures.size(), .texCoordIndex = 0
        };

        auto& gltf_texture = model.textures.emplace_back();
        gltf_texture.name = gltf_material.name;
        gltf_texture.samplerIndex = 0; // We'll all use a point-filtered wrapping sampler
        gltf_texture.imageIndex = model.images.size();

        // I write the actual images in the main function

        const auto image_filename = std::format("textures/{}.png", gltf_texture.name);
       
        auto& gltf_image = model.images.emplace_back();
        gltf_image.name = gltf_material.name;
        gltf_image.data = fastgltf::sources::URI{
            .uri = fastgltf::URI{image_filename}, .mimeType = fastgltf::MimeType::PNG
        };
    }

    auto positions = std::vector<uint8_t>{};
    positions.reserve(666 * 4 * sizeof(glm::vec3));
    auto normals = std::vector<uint8_t>{};
    normals.reserve(666 * 4 * sizeof(glm::vec3));
    auto texcoords = std::vector<uint8_t>{};
    texcoords.reserve(666 * 4 * sizeof(glm::vec2));
    auto indices = std::vector<uint8_t>{};
    indices.reserve(666 * 6 * sizeof(uint32_t));

    // Don't create buffer views for the buffers yet. We'll have buffer views that cover the whole
    // buffer, so we need to know the final size of the buffer
    // The views will be in the same order as the buffers, so we can construct the accessors

    for (const auto& sector : map.sectors) {
        scene.nodeIndices.emplace_back(model.nodes.size());

        const auto rotation_quat = glm::angleAxis(glm::radians(-90.f), glm::vec3{1, 0, 0});

        auto& node = model.nodes.emplace_back();
        node.name = name;
        node.meshIndex = model.meshes.size();
        node.transform = fastgltf::Node::TRS{
            .translation = {0, 0, 0},
            .rotation = {rotation_quat.x, rotation_quat.y, rotation_quat.z, rotation_quat.w},
            .scale = {1, 1, 1},
        };

        auto& mesh = model.meshes.emplace_back();
        mesh.name = std::format("{} Sector {}", name, model.meshes.size() - 1);

        for (const auto& face : sector.faces) {
            auto& primitive = mesh.primitives.emplace_back();
            primitive.type = fastgltf::PrimitiveType::Triangles;

            // Positions
            primitive.attributes.emplace_back("POSITION", model.accessors.size());
            auto& position_accessor = model.accessors.emplace_back();
            position_accessor.bufferViewIndex = 1;
            position_accessor.byteOffset = positions.size();
            position_accessor.componentType = fastgltf::ComponentType::Float;
            position_accessor.count = 4;
            position_accessor.type = fastgltf::AccessorType::Vec3;

            const auto min_x = glm::min(
                glm::min(face.vertices[0].position.x, face.vertices[1].position.x),
                glm::min(face.vertices[2].position.x, face.vertices[3].position.x)
            );
            const auto min_y = glm::min(
                glm::min(face.vertices[0].position.y, face.vertices[1].position.y),
                glm::min(face.vertices[2].position.y, face.vertices[3].position.y)
            );
            const auto min_z = glm::min(
                glm::min(face.vertices[0].position.z, face.vertices[1].position.z),
                glm::min(face.vertices[2].position.z, face.vertices[3].position.z)
            );
            const auto max_x = glm::max(
                glm::max(face.vertices[0].position.x, face.vertices[1].position.x),
                glm::max(face.vertices[2].position.x, face.vertices[3].position.x)
            );
            const auto max_y = glm::max(
                glm::max(face.vertices[0].position.y, face.vertices[1].position.y),
                glm::max(face.vertices[2].position.y, face.vertices[3].position.y)
            );
            const auto max_z = glm::max(
                glm::max(face.vertices[0].position.z, face.vertices[1].position.z),
                glm::max(face.vertices[2].position.z, face.vertices[3].position.z)
            );

            position_accessor.min = FASTGLTF_STD_PMR_NS::vector<double>{min_x, min_y, min_z};
            position_accessor.max = FASTGLTF_STD_PMR_NS::vector<double>{max_x, max_y, max_z};


            write_data_to_buffer<glm::vec3>(
                positions, std::array{
                    face.vertices[0].position,
                    face.vertices[1].position,
                    face.vertices[2].position,
                    face.vertices[3].position,
                }
            );

            // Normals
            primitive.attributes.emplace_back("NORMAL", model.accessors.size());
            auto& normal_accessor = model.accessors.emplace_back();
            normal_accessor.bufferViewIndex = 2;
            normal_accessor.byteOffset = normals.size();
            normal_accessor.componentType = fastgltf::ComponentType::Float;
            normal_accessor.count = 4;
            normal_accessor.type = fastgltf::AccessorType::Vec3;

            write_data_to_buffer<glm::vec3>(
                normals, std::array{
                    face.vertices[0].normal,
                    face.vertices[1].normal,
                    face.vertices[2].normal,
                    face.vertices[3].normal,
                }
            );

            // Texcoords
            primitive.attributes.emplace_back("TEXCOORD_0", model.accessors.size());
            auto& texcoord_accessor = model.accessors.emplace_back();
            texcoord_accessor.bufferViewIndex = 3;
            texcoord_accessor.byteOffset = texcoords.size();
            texcoord_accessor.componentType = fastgltf::ComponentType::Float;
            texcoord_accessor.count = 4;
            texcoord_accessor.type = fastgltf::AccessorType::Vec2;

            write_data_to_buffer<glm::vec2>(
                texcoords, std::array{
                    face.vertices[0].texcoord,
                    face.vertices[1].texcoord,
                    face.vertices[2].texcoord,
                    face.vertices[3].texcoord,
                }
            );

            // Indices
            primitive.indicesAccessor = model.accessors.size();
            auto& indices_accessor = model.accessors.emplace_back();
            indices_accessor.bufferViewIndex = 0;
            indices_accessor.byteOffset = indices.size();
            indices_accessor.componentType = fastgltf::ComponentType::UnsignedInt;
            indices_accessor.count = 6;
            indices_accessor.type = fastgltf::AccessorType::Scalar;

            // 0 1 2 3 2 1
            // Same for all faces
            write_data_to_buffer<uint32_t>(indices, std::array{0u, 1u, 2u, 3u, 2u, 1u});

            // Material. We create one material for each unique MapTexture, so there's a 1:1 relationship between
            // MapTexture indices and material indices
            primitive.materialIndex = face.texture_index;
        }
    }

    auto& indices_buffer_view = model.bufferViews.emplace_back();
    indices_buffer_view.name = "Indices Buffer View";
    indices_buffer_view.bufferIndex = 0;
    indices_buffer_view.byteOffset = 0;
    indices_buffer_view.byteLength = indices.size();
    indices_buffer_view.target = fastgltf::BufferTarget::ElementArrayBuffer; // lmao

    auto& positions_buffer_view = model.bufferViews.emplace_back();
    positions_buffer_view.name = "Positions Buffer View";
    positions_buffer_view.bufferIndex = 1;
    positions_buffer_view.byteOffset = 0;
    positions_buffer_view.byteLength = positions.size();
    positions_buffer_view.byteStride = sizeof(glm::vec3);
    positions_buffer_view.target = fastgltf::BufferTarget::ArrayBuffer;

    auto& normals_buffer_view = model.bufferViews.emplace_back();
    normals_buffer_view.name = "Normals Buffer View";
    normals_buffer_view.bufferIndex = 2;
    normals_buffer_view.byteOffset = 0;
    normals_buffer_view.byteLength = normals.size();
    normals_buffer_view.byteStride = sizeof(glm::vec3);
    normals_buffer_view.target = fastgltf::BufferTarget::ArrayBuffer;

    auto& texcoords_buffer_view = model.bufferViews.emplace_back();
    texcoords_buffer_view.name = "Texcoords Buffer View";
    texcoords_buffer_view.bufferIndex = 3;
    texcoords_buffer_view.byteOffset = 0;
    texcoords_buffer_view.byteLength = texcoords.size();
    texcoords_buffer_view.byteStride = sizeof(glm::vec2);
    texcoords_buffer_view.target = fastgltf::BufferTarget::ArrayBuffer;

    // Buffers for all the attributes, hopefully in a format that's easy to mutate
    model.buffers.resize(4);
    auto& indices_buffer = model.buffers[0];
    indices_buffer.byteLength = indices.size();
    indices_buffer.name = "Indices buffer";
    indices_buffer.data = fastgltf::sources::Vector{.bytes = indices};

    auto& positions_buffer = model.buffers[1];
    positions_buffer.byteLength = positions.size();
    positions_buffer.name = "Positions buffer";
    positions_buffer.data = fastgltf::sources::Vector{.bytes = positions};

    auto& normals_buffer = model.buffers[2];
    normals_buffer.byteLength = normals.size();
    normals_buffer.name = "Normals buffer";
    normals_buffer.data = fastgltf::sources::Vector{.bytes = normals};

    auto& texcoords_buffer = model.buffers[3];
    texcoords_buffer.byteLength = texcoords.size();
    texcoords_buffer.name = "Texcoords buffer";
    texcoords_buffer.data = fastgltf::sources::Vector{.bytes = texcoords};

    return model;
}
