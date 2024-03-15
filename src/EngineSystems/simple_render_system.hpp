/*************************************************
Render System Class:
1. pipeline
2. pipeline layout
3. simple push constant struct (temp constant buffer)
4. how to render the game objects

This system typically renders all game objects for now
We can have multiple render systems to render game objects in different ways.
Anything acts on game objects is defined as system.
It is application's resbonsibility to check whether an object is compatible to a system (as ECS)
*************************************************/
#pragma once

#include "Vk/lve_pipeline.hpp"
#include "Vk/lve_device.hpp"
#include "Vk/vk_shader_effect.hpp"
#include "EngineCore/frame_info.hpp"
#include "EngineCore/texture_manager.hpp"

// std
#include <memory>

namespace EngineSystem
{

    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(Vk::LveDevice& device, Vk::DescriptorLayoutCache& descriptorLayoutCache, VkRenderPass renderPass, EngineCore::TextureManager& textureManager, Vk::DescriptorAllocator& descriptorAllocator);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        void renderGameObjects(EngineCore::FrameInfo& frameInfo);

        void createDescriptorSetPerFrame(const std::string& name, VkDescriptorBufferInfo bufferInfo, VkShaderStageFlags stageFlags);
        void createDescriptorSetPerFrame(const std::string& name, VkDescriptorImageInfo imageInfo, VkShaderStageFlags stageFlags);
        void finishCreateDescriptorSetPerFrame();


    private:
        void createPipeline(VkRenderPass renderPass);

        void bindDescriptorSetsPerFrame(VkCommandBuffer commandBuffer);

        Vk::LveDevice& lveDevice;
        
        Vk::DescriptorAllocator& descriptorAllocator;
        Vk::DescriptorLayoutCache& descriptorLayoutCache;

        Vk::DescriptorBuilder descriptorBuilderPerFrame;
        VkDescriptorSet descriptorSetsPerFrame;
        
        Vk::ShaderEffect shaderEffect;
        std::unique_ptr<Vk::LvePipeline> lvePipeline;

        EngineCore::TextureManager& textureManager;
    };

}