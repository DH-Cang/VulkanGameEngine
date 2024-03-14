#include "point_light_system.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <map>

namespace EngineSystem
{
    struct PointLightPerObjectData
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(Vk::LveDevice& device, Vk::DescriptorLayoutCache& descriptorLayoutCache, VkRenderPass renderPass):
        lveDevice{device},
        descriptorAllocator(device.device()),
        descriptorLayoutCache(descriptorLayoutCache),
        descriptorBuilderPerFrame(descriptorLayoutCache, descriptorAllocator),
        shaderEffect(device.device(), descriptorLayoutCache, 
        "./build/ShaderBin/point_light.vert.spv", 
        "./build/ShaderBin/point_light.frag.spv")
    {
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem()
    {
    }


    void PointLightSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(shaderEffect.getPipelineLayout() != nullptr && "Cannot create pipeline before pipeline layout");

        Vk::PipelineConfigInfo pipelineConfig{};
        Vk::LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        Vk::LvePipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = shaderEffect.getPipelineLayout();
        lvePipeline = std::make_unique<Vk::LvePipeline>(
            lveDevice, 
            shaderEffect.getVertShaderModule(), 
            shaderEffect.getFragShaderModule(), 
            pipelineConfig
        );
    }

    void PointLightSystem::update(EngineCore::FrameInfo& frameInfo, EngineCore::GlobalUbo& ubo)
    {
        auto rotateLight = glm::rotate(
                glm::mat4(1.0f),
                frameInfo.frameTime,
                {0.0f, -1.0f, 0.0f}
        );
        int lightIndex = 0;
        for(auto& kv : frameInfo.gameObjects)
        {
            auto& obj = kv.second;
            if(obj.pointLight == nullptr) continue;

            assert(lightIndex < MAX_LIGHTS && "point light number limits!");
            
            // update light position
            obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.0f));

            // copy light to ubo
            ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.0f);
            ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);

            lightIndex++;
        }
        ubo.numLights = lightIndex;
    }


    void PointLightSystem::render(EngineCore::FrameInfo& frameInfo)
    {
        // sort lights
        std::map<float, EngineCore::GameObject::id_t> sorted;
        for(auto& kv : frameInfo.gameObjects)
        {
            auto& obj = kv.second;
            if(obj.pointLight == nullptr) continue;

            // calculate distance
            auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
            float disSquared = glm::dot(offset, offset);
            sorted[disSquared] = obj.getId();
        }

        // render
        lvePipeline->bind(frameInfo.commandBuffer);

        bindDescriptorSetsPerFrame(frameInfo.commandBuffer);

        // iterate through sorted lights in reverse order
        for(auto it = sorted.rbegin(); it != sorted.rend(); it++)
        {
            // use game obj id to find light obj
            auto& obj = frameInfo.gameObjects.at(it->second);

            if(obj.transform.is_descriptor_allocated == false)
            {
                if(obj.transform.ubo == nullptr)
                {
                    obj.transform.ubo = std::make_shared<Vk::LveBuffer>(
                        lveDevice,
                        sizeof(PointLightPerObjectData),
                        1,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                    obj.transform.ubo->map();
                }
                auto descriptorInfo = obj.transform.ubo->descriptorInfo();
                Vk::DescriptorBuilder builder(descriptorLayoutCache, descriptorAllocator);
                builder.bind_buffer(0, &descriptorInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT).build(obj.transform.descriptorSet);
                obj.transform.is_descriptor_allocated = true;
            }

            PointLightPerObjectData perObjectUboData
            {
                glm::vec4(obj.transform.translation, 1.0f),
                glm::vec4(obj.color, obj.pointLight->lightIntensity),
                obj.transform.scale.x
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

            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }


    void PointLightSystem::createDescriptorSetPerFrame(const std::string& name, VkDescriptorBufferInfo bufferInfo, VkShaderStageFlags stageFlags)
    {
        const auto setAndBinding = shaderEffect.getSetAndBinding(name);
        assert(setAndBinding.setId == 0); // per frame set can only be set0
        descriptorBuilderPerFrame.bind_buffer(
            setAndBinding.bindingId, 
            &bufferInfo, 
            setAndBinding.type, 
            stageFlags);
    }

    void PointLightSystem::createDescriptorSetPerFrame(const std::string& name, VkDescriptorImageInfo imageInfo, VkShaderStageFlags stageFlags)
    {
        const auto setAndBinding = shaderEffect.getSetAndBinding(name);
        assert(setAndBinding.setId == 0); // per frame set can only be set0 and set1
        descriptorBuilderPerFrame.bind_image(
            setAndBinding.bindingId, 
            &imageInfo, 
            setAndBinding.type, 
            stageFlags);
    }

    void PointLightSystem::finishCreateDescriptorSetPerFrame()
    {
        descriptorBuilderPerFrame.build(descriptorSetsPerFrame);
    }

    void PointLightSystem::bindDescriptorSetsPerFrame(VkCommandBuffer commandBuffer)
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

}