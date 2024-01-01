#include "gltf_export.hpp"

#include <format>
#include <tiny_gltf.h>

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtc/quaternion.hpp>

template <typename DataType>
void write_data_to_buffer(std::vector<uint8_t>& buffer, const std::span<const DataType> data)
{
    const auto dest_offset = buffer.size();
    const auto num_bytes = data.size() * sizeof(DataType);
    buffer.resize(buffer.size() + num_bytes);
    auto* dest_ptr = buffer.data() + dest_offset;
    std::memcpy(dest_ptr, data.data(), num_bytes);
}

tinygltf::Model export_to_gltf(const std::string_view name, const Map& map)
{
    auto model = tinygltf::Model{};
    model.defaultScene = 0;
    model.asset.generator = "wad2gltf";

    auto& scene = model.scenes.emplace_back();
    scene.name = std::string{name};

    auto& sampler = model.samplers.emplace_back();
    sampler.magFilter = TINYGLTF_TEXTURE_FILTER_NEAREST;
    sampler.minFilter = TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST;

    // Create materials for all the map textures. We'll shrimply create a non-PBR material for each one
    for(const auto& texture : map.textures) {
        auto& gltf_material = model.materials.emplace_back();
        gltf_material.name = texture.info.name.to_string();
        // TODO: Use masked alpha for images with transparency
        gltf_material.pbrMetallicRoughness.baseColorFactor = { 1, 1, 1, 1 };
        gltf_material.pbrMetallicRoughness.baseColorTexture.index = static_cast<int>(model.textures.size());

        auto& gltf_texture = model.textures.emplace_back();
        gltf_texture.name = gltf_material.name;
        gltf_texture.sampler = 0;   // We'll all use a point-filtered wrapping sampler
        gltf_texture.source = static_cast<int>(model.images.size());

        auto& gltf_image = model.images.emplace_back();
        gltf_image.name = gltf_material.name;
        gltf_image.width = texture.info.width;
        gltf_image.height = texture.info.height;
        gltf_image.component = 1;   // One component for now, we'll deal with palettes later
        gltf_image.bits = 8;
        gltf_image.pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
        gltf_image.image = texture.pixels;
        gltf_image.mimeType = "image/png";
    }

    // Buffers for all the attributes, hopefully in a format that's easy to mutate
    model.buffers.resize(4);
    auto& indices_buffer = model.buffers[0];
    indices_buffer.name = "Indices buffer";
    indices_buffer.data = {};
    indices_buffer.uri = {};
    auto& positions_buffer = model.buffers[1];
    positions_buffer.name = "Positions buffer";
    positions_buffer.data = {};
    positions_buffer.uri = {};
    auto& normals_buffer = model.buffers[2];
    normals_buffer.name = "Normals buffer";
    normals_buffer.data = {};
    normals_buffer.uri = {};
    auto& texcoords_buffer = model.buffers[3];
    texcoords_buffer.name = "Texcoords buffer";
    texcoords_buffer.data = {};
    texcoords_buffer.uri = {};

    // Don't create buffer views for the buffers yet. We'll have buffer views that cover the whole
    // buffer, so we need to know the final size of the buffer
    // The views will be in the same order as the buffers, so we can construct the accessors

    for (const auto& sector : map.sectors)
    {
        scene.nodes.emplace_back(static_cast<int>(model.nodes.size()));

        const auto rotation_quat = glm::angleAxis(glm::radians(-90.f), glm::vec3{1, 0, 0});

        auto& node = model.nodes.emplace_back();
        node.name = std::string{ name };
        node.mesh = static_cast<int>(model.meshes.size());
        node.rotation = { rotation_quat.x, rotation_quat.y, rotation_quat.z, rotation_quat.w };
        node.scale = { 1, 1, 1 };
        node.translation = { 0, 0, 0 };

        auto& mesh = model.meshes.emplace_back();
        mesh.name = std::format( "{} Sector {}", name, model.meshes.size() - 1);

        for (const auto& face : sector.faces)
        {
            auto& primitive = mesh.primitives.emplace_back();
            primitive.mode = TINYGLTF_MODE_TRIANGLES;

            // Positions
            primitive.attributes.emplace("POSITION", static_cast<int>(model.accessors.size()));
            auto& position_accessor = model.accessors.emplace_back();
            position_accessor.bufferView = 1;
            position_accessor.byteOffset = positions_buffer.data.size();
            position_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            position_accessor.count = 4;
            position_accessor.type = TINYGLTF_TYPE_VEC3;

            const auto min_x = glm::min(glm::min(face.vertices[0].position.x, face.vertices[1].position.x),
                                        glm::min(face.vertices[2].position.x, face.vertices[3].position.x));
            const auto min_y = glm::min(glm::min(face.vertices[0].position.y, face.vertices[1].position.y),
                                        glm::min(face.vertices[2].position.y, face.vertices[3].position.y));
            const auto min_z = glm::min(glm::min(face.vertices[0].position.z, face.vertices[1].position.z),
                                        glm::min(face.vertices[2].position.z, face.vertices[3].position.z));
            const auto max_x = glm::max(glm::max(face.vertices[0].position.x, face.vertices[1].position.x),
                                        glm::max(face.vertices[2].position.x, face.vertices[3].position.x));
            const auto max_y = glm::max(glm::max(face.vertices[0].position.y, face.vertices[1].position.y),
                                        glm::max(face.vertices[2].position.y, face.vertices[3].position.y));
            const auto max_z = glm::max(glm::max(face.vertices[0].position.z, face.vertices[1].position.z),
                                        glm::max(face.vertices[2].position.z, face.vertices[3].position.z));

            position_accessor.minValues = {min_x, min_y, min_z};
            position_accessor.maxValues = {max_x, max_y, max_z};

            write_data_to_buffer<glm::vec3>(positions_buffer.data, std::array{
                                                face.vertices[0].position,
                                                face.vertices[1].position,
                                                face.vertices[2].position,
                                                face.vertices[3].position,
                                            });

            // Normals
            primitive.attributes.emplace("NORMAL", static_cast<int>(model.accessors.size()));
            auto& normal_accessor = model.accessors.emplace_back();
            normal_accessor.bufferView = 2;
            normal_accessor.byteOffset = normals_buffer.data.size();
            normal_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            normal_accessor.count = 4;
            normal_accessor.type = TINYGLTF_TYPE_VEC3;

            write_data_to_buffer<glm::vec3>(normals_buffer.data, std::array{
                                                face.vertices[0].normal,
                                                face.vertices[1].normal,
                                                face.vertices[2].normal,
                                                face.vertices[3].normal,
                                            });

            // Texcoords
            primitive.attributes.emplace("TEXCOORD_0", static_cast<int>(model.accessors.size()));
            auto& texcoord_accessor = model.accessors.emplace_back();
            texcoord_accessor.bufferView = 3;
            texcoord_accessor.byteOffset = texcoords_buffer.data.size();
            texcoord_accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            texcoord_accessor.count = 4;
            texcoord_accessor.type = TINYGLTF_TYPE_VEC2;

            write_data_to_buffer<glm::vec2>(texcoords_buffer.data, std::array{
                                                face.vertices[0].texcoord,
                                                face.vertices[1].texcoord,
                                                face.vertices[2].texcoord,
                                                face.vertices[3].texcoord,
                                            });

            // Indices
            primitive.indices = static_cast<int>(model.accessors.size());
            auto& indices_accessor = model.accessors.emplace_back();
            indices_accessor.bufferView = 0;
            indices_accessor.byteOffset = indices_buffer.data.size();
            indices_accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
            indices_accessor.count = 6;
            indices_accessor.type = TINYGLTF_TYPE_SCALAR;

            // 0 1 2 3 2 1
            // Same for all faces
            write_data_to_buffer<uint32_t>(indices_buffer.data, std::array{0u, 1u, 2u, 3u, 2u, 1u});

            // Material. We create one material for each unique MapTexture, so there's a 1:1 relationship between
            // MapTexture indices and material indices
            primitive.material = static_cast<int>(face.texture_index);
        }
    }

    auto& indices_buffer_view = model.bufferViews.emplace_back();
    indices_buffer_view.name = "Indices Buffer View";
    indices_buffer_view.buffer = 0;
    indices_buffer_view.byteOffset = 0;
    indices_buffer_view.byteLength = indices_buffer.data.size();
    indices_buffer_view.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER; // lmao

    auto& positions_buffer_view = model.bufferViews.emplace_back();
    positions_buffer_view.name = "Positions Buffer View";
    positions_buffer_view.buffer = 1;
    positions_buffer_view.byteOffset = 0;
    positions_buffer_view.byteLength = positions_buffer.data.size();
    positions_buffer_view.byteStride = sizeof(glm::vec3);
    positions_buffer_view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    auto& normals_buffer_view = model.bufferViews.emplace_back();
    normals_buffer_view.name = "Normals Buffer View";
    normals_buffer_view.buffer = 2;
    normals_buffer_view.byteOffset = 0;
    normals_buffer_view.byteLength = normals_buffer.data.size();
    normals_buffer_view.byteStride = sizeof(glm::vec3);
    normals_buffer_view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    auto& texcoords_buffer_view = model.bufferViews.emplace_back();
    texcoords_buffer_view.name = "Texcoords Buffer View";
    texcoords_buffer_view.buffer = 3;
    texcoords_buffer_view.byteOffset = 0;
    texcoords_buffer_view.byteLength = texcoords_buffer.data.size();
    texcoords_buffer_view.byteStride = sizeof(glm::vec2);
    texcoords_buffer_view.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    return model;
}
