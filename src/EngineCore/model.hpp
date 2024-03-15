#pragma once

#include "material.hpp"
#include "texture_manager.hpp"

#include "Vk/lve_model.hpp"
#include "VK/vk_descriptor.hpp"

// std
#include <memory>


namespace EngineCore
{
    class Model
    {
    public:
        Model(Vk::LveDevice& device);
        ~Model() = default;
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        static std::unique_ptr<Model> createModelFromFile(Vk::LveDevice& device, TextureManager& textureManager, const std::string& filePath, const std::string& mtlBasePath);
        void bindAndDraw(VkCommandBuffer commandBuffer, Vk::DescriptorAllocator& descriptorAllocator, Vk::DescriptorLayoutCache& descriptorLayoutCache, VkPipelineLayout pipelineLayout, TextureManager& textureManager);
        
    private:
        std::vector<std::unique_ptr<Vk::LveModel>> lveModels;
        std::vector<Material> materials;

        Vk::LveDevice& lveDevice;
        
    };
}
