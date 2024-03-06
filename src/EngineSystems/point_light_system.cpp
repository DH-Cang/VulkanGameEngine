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
    struct PointLightPushConstants
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };


    PointLightSystem::PointLightSystem(Vk::LveDevice& device, VkRenderPass renderPass, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts):
        lveDevice{device}
    {
        createPipelineLayout(descriptorSetLayouts);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem()
    {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void PointLightSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        Vk::PipelineConfigInfo pipelineConfig{};
        Vk::LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        Vk::LvePipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<Vk::LvePipeline>(
            lveDevice, 
            "./build/ShaderBin/point_light.vert.spv", 
            "./build/ShaderBin/point_light.frag.spv", 
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

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            frameInfo.descriptorSets.size(),
            frameInfo.descriptorSets.data(),
            0,
            nullptr
        );

        // iterate through sorted lights in reverse order
        for(auto it = sorted.rbegin(); it != sorted.rend(); it++)
        {
            // use game obj id to find light obj
            auto& obj = frameInfo.gameObjects.at(it->second);

            PointLightPushConstants push{};
            push.position = glm::vec4(obj.transform.translation, 1.0f);
            push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            push.radius = obj.transform.scale.x;

            vkCmdPushConstants(
                frameInfo.commandBuffer, 
                pipelineLayout, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                0,
                sizeof(PointLightPushConstants), 
                &push
            );
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }

    }

};