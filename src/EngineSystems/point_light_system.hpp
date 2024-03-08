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
#include "Vk/lve_shader.hpp"
#include "EngineCore/frame_info.hpp"

// std
#include <memory>

namespace EngineSystem
{

    class PointLightSystem
    {
    public:
        PointLightSystem(Vk::LveDevice& device, VkRenderPass renderPass);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem& operator=(const PointLightSystem&) = delete;

        void update(EngineCore::FrameInfo& frameInfo, EngineCore::GlobalUbo& ubo);
        void render(EngineCore::FrameInfo& frameInfo);

        void writeDescriptorToSets(const std::string& name, VkDescriptorBufferInfo bufferInfo, Vk::LveDescriptorPool& descriptorPool);
        void writeDescriptorToSets(const std::string& name, VkDescriptorImageInfo imageInfo, Vk::LveDescriptorPool& descriptorPool);
        void finishWriteDescriptor();

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);
        std::unique_ptr<Vk::LveShader> createShader(const std::string& shaderFilePath);

        void bindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
        void printDescriptorSignatures()
        {
            for(auto& iter : descriptorSignature )
            {
                printf("%s, set %d, binding %d\n", iter.first.c_str(), iter.second.setId, iter.second.bindingId);
            }
        }

        Vk::LveDevice& lveDevice;
        
        std::unique_ptr<Vk::LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;


        std::unique_ptr<Vk::LveShader> vertShader;
        std::unique_ptr<Vk::LveShader> fragShader;
        

        std::unordered_map<std::string, Vk::LveShader::SetAndBinding> descriptorSignature; // shader reflection data goes into this obj
        std::vector<std::unique_ptr<Vk::LveDescriptorSetLayout>> descriptorSetLayouts; // after shader is created, this obj is populated with descriptor set layout

        std::vector<std::shared_ptr<Vk::LveDescriptorWriter>> descriptorWriters; // write into descriptor sets
        std::vector<VkDescriptorSet> descriptorSets;
    };

}