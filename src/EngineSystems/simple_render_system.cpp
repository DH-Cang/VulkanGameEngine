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
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
        //alignas(16) glm::vec3 color; // make sure the memory align as 16 bytes, to match the data alignment in shader
    };

    SimpleRenderSystem::SimpleRenderSystem(Vk::LveDevice& device, Vk::DescriptorLayoutCache& descriptorLayoutCache, VkRenderPass renderPass):
        lveDevice{device},
        descriptorAllocator(device.device()),
        descriptorLayoutCache(descriptorLayoutCache),
        shaderEffect(device.device(), 
        descriptorLayoutCache, 
        "./build/ShaderBin/simple_shader.vert.spv", 
        "./build/ShaderBin/simple_shader.frag.spv")
    {
        createPipeline(renderPass);

        Vk::DescriptorBuilder builder(descriptorLayoutCache, descriptorAllocator);
        descriptorBuilderPerFrame.resize(2, builder);
        descriptorSetsPerFrame.resize(2);
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

            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer, 
                shaderEffect.getPipelineLayout(), 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                0, 
                sizeof(SimplePushConstantData), 
                &push);
            
            obj.model->bindAndDraw(frameInfo.commandBuffer, descriptorAllocator, descriptorLayoutCache, shaderEffect.getPipelineLayout());
        }
    }

    void SimpleRenderSystem::createDescriptorSetPerFrame(const std::string& name, VkDescriptorBufferInfo bufferInfo, VkShaderStageFlags stageFlags)
    {
        const auto setAndBinding = shaderEffect.getSetAndBinding(name);
        assert(setAndBinding.setId <= 1); // per frame set can only be set0 and set1
        descriptorBuilderPerFrame[setAndBinding.setId].bind_buffer(
            setAndBinding.bindingId, 
            &bufferInfo, 
            setAndBinding.type, 
            stageFlags);
    }

    void SimpleRenderSystem::createDescriptorSetPerFrame(const std::string& name, VkDescriptorImageInfo imageInfo, VkShaderStageFlags stageFlags)
    {
        const auto setAndBinding = shaderEffect.getSetAndBinding(name);
        assert(setAndBinding.setId <= 1); // per frame set can only be set0 and set1
        descriptorBuilderPerFrame[setAndBinding.setId].bind_image(
            setAndBinding.bindingId, 
            &imageInfo, 
            setAndBinding.type, 
            stageFlags);
    }

    void SimpleRenderSystem::finishCreateDescriptorSetPerFrame()
    {
        for(int i=0; i<descriptorBuilderPerFrame.size(); i++)
        {
            descriptorBuilderPerFrame[i].build(descriptorSetsPerFrame[i]);
        }
    }

    void SimpleRenderSystem::bindDescriptorSetsPerFrame(VkCommandBuffer commandBuffer)
    {
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            shaderEffect.getPipelineLayout(),
            0,
            descriptorSetsPerFrame.size(),
            descriptorSetsPerFrame.data(),
            0,
            nullptr
        );
    }

};