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

// std
#include <memory>

namespace lve {

    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

        void renderGameObjects(
            VkCommandBuffer commandBuffer, 
            std::vector<LveGameObject>& gameObjects, 
            const LveCamera& camera);

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        LveDevice& lveDevice;
        
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };

}