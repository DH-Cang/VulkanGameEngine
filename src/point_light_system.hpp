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

#include "lve_camera.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_frame_info.hpp"

// std
#include <memory>

namespace lve {

    class PointLightSystem
    {
    public:
        PointLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem& operator=(const PointLightSystem&) = delete;

        void render(FrameInfo& frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        LveDevice& lveDevice;
        
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };

}