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
    struct PerObjectUboData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
        //alignas(16) glm::vec3 color; // make sure the memory align as 16 bytes, to match the data alignment in shader
    };

    SimpleRenderSystem::SimpleRenderSystem(Vk::LveDevice& device, Vk::DescriptorLayoutCache& descriptorLayoutCache, VkRenderPass renderPass, EngineCore::TextureManager& textureManager):
        lveDevice{device},
        descriptorAllocator(device.device()),
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

            if(obj.transform.is_descriptor_allocated == false)
            {
                if(obj.transform.ubo == nullptr)
                {
                    obj.transform.ubo = std::make_shared<Vk::LveBuffer>(
                        lveDevice,
                        sizeof(PerObjectUboData),
                        1,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                    obj.transform.ubo->map();
                }
                auto descriptorInfo = obj.transform.ubo->descriptorInfo();
                Vk::DescriptorBuilder builder(descriptorLayoutCache, descriptorAllocator);
                builder.bind_buffer(0, &descriptorInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).build(obj.transform.descriptorSet);
                obj.transform.is_descriptor_allocated = true;
            }

            PerObjectUboData perObjectUboData
            {
                obj.transform.mat4(),
                obj.transform.normalMatrix()
            };
            obj.transform.ubo->writeToBuffer(&perObjectUboData);

            vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                shaderEffect.getPipelineLayout(),
                1,
                1,
                &obj.transform.descriptorSet,
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