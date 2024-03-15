#include "simple_render_system.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>

namespace EngineSystem
{

    SimpleRenderSystem::SimpleRenderSystem(Vk::LveDevice& device, Vk::DescriptorLayoutCache& descriptorLayoutCache, VkRenderPass renderPass, EngineCore::TextureManager& textureManager, Vk::DescriptorAllocator& descriptorAllocator):
        lveDevice{device},
        descriptorAllocator(descriptorAllocator),
        descriptorLayoutCache(descriptorLayoutCache),
        descriptorBuilderPerFrame(descriptorLayoutCache, descriptorAllocator),
        shaderEffect(device.device(), descriptorLayoutCache, 
        "./build/ShaderBin/simple_shader.vert.spv", 
        "./build/ShaderBin/simple_shader.frag.spv"),
        textureManager(textureManager)
    {
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem()
    {
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(shaderEffect.getPipelineLayout() != nullptr && "Cannot create pipeline before pipeline layout");

        Vk::PipelineConfigInfo pipelineConfig{};
        Vk::LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = shaderEffect.getPipelineLayout();
        lvePipeline = std::make_unique<Vk::LvePipeline>(
            lveDevice, 
            shaderEffect.getVertShaderModule(), 
            shaderEffect.getFragShaderModule(), 
            pipelineConfig
        );
    }

    void SimpleRenderSystem::renderGameObjects(EngineCore::FrameInfo& frameInfo)
    {
        lvePipeline->bind(frameInfo.commandBuffer);

        bindDescriptorSetsPerFrame(frameInfo.commandBuffer);

        for(auto& kv : frameInfo.gameObjects)
        {
            auto& obj = kv.second;
            if(obj.model == nullptr) continue;

            obj.updateSimpleObject();

            auto descriptorSet = obj.getSimpleDescriptor();
            vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                shaderEffect.getPipelineLayout(),
                1,
                1,
                &descriptorSet,
                0,
                nullptr
            );

            obj.model->bindAndDraw(frameInfo.commandBuffer, descriptorAllocator, descriptorLayoutCache, shaderEffect.getPipelineLayout(), textureManager);
        }
    }

    void SimpleRenderSystem::createDescriptorSetPerFrame(const std::string& name, VkDescriptorBufferInfo bufferInfo, VkShaderStageFlags stageFlags)
    {
        const auto setAndBinding = shaderEffect.getSetAndBinding(name);
        assert(setAndBinding.setId == 0); // per frame set can only be set0
        descriptorBuilderPerFrame.bind_buffer(
            setAndBinding.bindingId, 
            &bufferInfo, 
            setAndBinding.type, 
            stageFlags);
    }

    void SimpleRenderSystem::createDescriptorSetPerFrame(const std::string& name, VkDescriptorImageInfo imageInfo, VkShaderStageFlags stageFlags)
    {
        const auto setAndBinding = shaderEffect.getSetAndBinding(name);
        assert(setAndBinding.setId == 0); // per frame set can only be set0 and set1
        descriptorBuilderPerFrame.bind_image(
            setAndBinding.bindingId, 
            &imageInfo, 
            setAndBinding.type, 
            stageFlags);
    }

    void SimpleRenderSystem::finishCreateDescriptorSetPerFrame()
    {
        descriptorBuilderPerFrame.build(descriptorSetsPerFrame);
    }

    void SimpleRenderSystem::bindDescriptorSetsPerFrame(VkCommandBuffer commandBuffer)
    {
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            shaderEffect.getPipelineLayout(),
            0,
            1,
            &descriptorSetsPerFrame,
            0,
            nullptr
        );
    }

};