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
#include "EngineCore/frame_info.hpp"

// std
#include <memory>

namespace EngineSystem
{

    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(Vk::LveDevice& device, VkRenderPass renderPass, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        void renderGameObjects(EngineCore::FrameInfo& frameInfo);

    private:
        void createPipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
        void createPipeline(VkRenderPass renderPass);

        Vk::LveDevice& lveDevice;
        
        std::unique_ptr<Vk::LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };

}