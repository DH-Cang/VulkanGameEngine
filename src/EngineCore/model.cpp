#include "model.hpp"

// libs
#include "ThirdParty\utility.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "ThirdParty\tiny_obj_loader.h"

// std
#include <stdexcept>
#include <filesystem>

namespace std
{
    template <>
    struct hash<Vk::LveModel::Vertex>
    {
        size_t operator()(Vk::LveModel::Vertex const& vertex) const
        {
            size_t seed = 0;
            Util::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}


namespace EngineCore
{
    Model::Model(Vk::LveDevice& device): lveDevice{device} {}

    void Model::bindAndDraw(VkCommandBuffer commandBuffer, Vk::DescriptorAllocator& descriptorAllocator, Vk::DescriptorLayoutCache& descriptorLayoutCache, VkPipelineLayout pipelineLayout, TextureManager& textureManager)
    {
        for(int i=0; i<lveModels.size(); i++)
        {
            auto& material = materials[i];

            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                2,
                1,
                &material.descriptorSet,
                0,
                nullptr
            );

            auto& model = lveModels[i];
            model->bind(commandBuffer);
            model->draw(commandBuffer);
        }
    }
    
    std::unique_ptr<Model> Model::createModelFromFile(
            Vk::LveDevice& device, 
            TextureManager& textureManager, 
            Vk::DescriptorAllocator& descriptorAllocator, 
            Vk::DescriptorLayoutCache& descriptorLayoutCache, 
            const std::string& objPath, 
            const std::string& mtlBasePath)
    {
        auto ret = std::make_unique<Model>(device);

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> obj_materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &obj_materials, &warn, &err, objPath.c_str(), mtlBasePath.c_str())) {
            throw std::runtime_error(warn + err);
        }
        assert(shapes.size() > 0 && obj_materials.size() > 0 && "obj file must be more than 0 shapes and materials");

        // for each material: load into temp materials
        auto materialNum = obj_materials.size();
        std::vector<Material> temp_materials(materialNum);
        for(size_t i = 0; i<materialNum; i++)
        {
            tinyobj::material_t& obj_material = obj_materials[i];
            Material& model_material = temp_materials[i];

            model_material.materialData.ambient[0] = obj_material.ambient[0];
            model_material.materialData.ambient[1] = obj_material.ambient[1];
            model_material.materialData.ambient[2] = obj_material.ambient[2];
            model_material.materialData.blinn_factor = 32.0f;

            if(obj_material.ambient_texname.empty() == false)
            {
                std::filesystem::path ambientTextureName = obj_material.ambient_texname;
                ambientTextureName = mtlBasePath / ambientTextureName;
                textureManager.addTexture(ambientTextureName.string());
                model_material.ambientTextureName = ambientTextureName.string();
            }
            if(obj_material.roughness_texname.empty() == false)
            {
                std::filesystem::path roughnessTextureName = obj_material.roughness_texname;
                roughnessTextureName = mtlBasePath / roughnessTextureName;
                textureManager.addTexture(roughnessTextureName.string());
                model_material.roughnessTextureName = roughnessTextureName.string();
            }
            if(obj_material.metallic_texname.empty() == false)
            {
                std::filesystem::path metallicTextureName = obj_material.metallic_texname;
                metallicTextureName = mtlBasePath / metallicTextureName;
                textureManager.addTexture(metallicTextureName.string());
                model_material.metallicTextureName = metallicTextureName.string();
            }
        }

        std::vector<Vk::LveModel::Builder> builder_array(materialNum); // for each material, it has a builder
        std::vector<std::unordered_map<Vk::LveModel::Vertex, uint32_t>> uniqueVertices_array(materialNum); // for each material, it maintains a unique vertices

        // gather faces that has the same material to form a submodel
        // for each shape
        for (size_t i = 0; i < shapes.size(); i++)
        {
            // for each face
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) 
            {
                size_t fnum = shapes[i].mesh.num_face_vertices[f];
                assert(fnum == 3); // assert that each face has 3 vertices

                int materialId = shapes[i].mesh.material_ids[f];
                auto& builder = builder_array[materialId];
                auto& uniqueVertices = uniqueVertices_array[materialId];

                // for each index
                for (size_t v = 0; v < fnum; v++)
                {
                    Vk::LveModel::Vertex vertex{};
                    tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];

                    if (idx.vertex_index >= 0)
                    {
                        vertex.position = {
                            attrib.vertices[3 * idx.vertex_index + 0],
                            attrib.vertices[3 * idx.vertex_index + 1],
                            attrib.vertices[3 * idx.vertex_index + 2],
                        };

                        vertex.color = {
                            attrib.colors[3 * idx.vertex_index + 0],
                            attrib.colors[3 * idx.vertex_index + 1],
                            attrib.colors[3 * idx.vertex_index + 2],
                        };
                    }

                    if (idx.normal_index >= 0)
                    {
                        vertex.normal = {
                            attrib.normals[3 * idx.normal_index + 0],
                            attrib.normals[3 * idx.normal_index + 1],
                            attrib.normals[3 * idx.normal_index + 2],
                        };
                    }

                    if (idx.texcoord_index >= 0)
                    {
                        vertex.uv = {
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            attrib.texcoords[2 * idx.texcoord_index + 1],
                        };
                    }

                    if (uniqueVertices.count(vertex) == 0)
                    {
                        uniqueVertices[vertex] = static_cast<uint32_t>(builder.vertices.size());
                        builder.vertices.push_back(vertex);
                    }
                    builder.indices.push_back(uniqueVertices[vertex]);
                }

                index_offset += fnum;
            }

        }

        // create submodel for each builder
        for(int i=0; i<materialNum; i++)
        {
            const auto& builder = builder_array[i];
            if(builder.indices.size() > 0 && builder.vertices.size() > 0)
            {
                ret->lveModels.push_back(std::make_unique<Vk::LveModel>(device, builder));
                ret->materials.push_back(temp_materials[i]);
                ret->materials.back().ubo = std::make_shared<Vk::LveBuffer>(
                    device,
                    sizeof(Material::Data),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                ret->materials.back().ubo->map();
                ret->materials.back().ubo->writeToBuffer(&ret->materials.back().materialData);

                auto descriptorInfo = ret->materials.back().ubo->descriptorInfo();
                Vk::DescriptorBuilder builder(descriptorLayoutCache, descriptorAllocator);
                builder.bind_buffer(0, &descriptorInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

                assert(ret->materials.back().ambientTextureName.empty() == false);
                auto ambientTextureInfo = textureManager.getTexture(ret->materials.back().ambientTextureName)->getDescriptorImageInfo();
                builder.bind_image(1, &ambientTextureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

                assert(ret->materials.back().metallicTextureName.empty() == false);
                auto matallicTextureInfo = textureManager.getTexture(ret->materials.back().metallicTextureName)->getDescriptorImageInfo();
                builder.bind_image(2, &matallicTextureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

                assert(ret->materials.back().roughnessTextureName.empty() == false);
                auto roughnessTextureInfo = textureManager.getTexture(ret->materials.back().roughnessTextureName)->getDescriptorImageInfo();
                builder.bind_image(3, &roughnessTextureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

                builder.build(ret->materials.back().descriptorSet);
            }
        }
        printf("Load %s, shapes num %d, material num %d\n", objPath.c_str(), ret->lveModels.size(), ret->materials.size());

        

        return ret;
    }
}