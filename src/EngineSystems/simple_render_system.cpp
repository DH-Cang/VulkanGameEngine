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

    SimpleRenderSystem::SimpleRenderSystem(Vk::LveDevice& device, VkRenderPass renderPass):
        lveDevice{device}
    {
        vertShader = createShader("./build/ShaderBin/simple_shader.vert.spv");
        fragShader = createShader("./build/ShaderBin/simple_shader.frag.spv");
        descriptorWriters.resize(descriptorSetLayouts.size());

        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem()
    {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    std::unique_ptr<Vk::LveShader> SimpleRenderSystem::createShader(const std::string& shaderFilePath)
    {
        return std::make_unique<Vk::LveShader>(lveDevice, descriptorSignature, descriptorSetLayouts, shaderFilePath);
    }

    void SimpleRenderSystem::createPipelineLayout()
    {
        std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts(descriptorSetLayouts.size());
        for(int i=0; i<vkDescriptorSetLayouts.size(); i++)
        {
            vkDescriptorSetLayouts[i] = descriptorSetLayouts[i]->getDescriptorSetLayout();
        }

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = vkDescriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        Vk::PipelineConfigInfo pipelineConfig{};
        Vk::LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<Vk::LvePipeline>(
            lveDevice, 
            *vertShader, 
            *fragShader, 
            pipelineConfig
        );
    }

    void SimpleRenderSystem::renderGameObjects(EngineCore::FrameInfo& frameInfo)
    {
        // render
        lvePipeline->bind(frameInfo.commandBuffer);

        bindDescriptorSets(frameInfo.commandBuffer, pipelineLayout);

        for(auto& kv : frameInfo.gameObjects)
        {
            auto& obj = kv.second;
            if(obj.model == nullptr) continue;

            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer, 
                pipelineLayout, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                0, 
                sizeof(SimplePushConstantData), 
                &push);
            
            obj.model->bindAndDraw(frameInfo.commandBuffer);
        }
    }

    void SimpleRenderSystem::writeDescriptorToSets(const std::string& name, VkDescriptorBufferInfo bufferInfo, Vk::LveDescriptorPool& descriptorPool)
    {
        const auto& descriptorRecord = descriptorSignature.find(name);
        assert(descriptorRecord != descriptorSignature.end());
        auto setId = descriptorRecord->second.setId;
        auto bindingId = descriptorRecord->second.bindingId;

        assert(descriptorWriters.size() == descriptorSetLayouts.size());
        if(descriptorWriters[setId] == nullptr)
        {
            descriptorWriters[setId] = std::make_shared<Vk::LveDescriptorWriter>(*descriptorSetLayouts[setId], descriptorPool);
        }
        descriptorWriters[setId]->writeBuffer(bindingId, &bufferInfo);
    }

    void SimpleRenderSystem::writeDescriptorToSets(const std::string& name, VkDescriptorImageInfo imageInfo, Vk::LveDescriptorPool& descriptorPool)
    {
        const auto& descriptorRecord = descriptorSignature.find(name);
        assert(descriptorRecord != descriptorSignature.end());
        auto setId = descriptorRecord->second.setId;
        auto bindingId = descriptorRecord->second.bindingId;

        assert(descriptorWriters.size() == descriptorSetLayouts.size());
        if(descriptorWriters[setId] == nullptr)
        {
            descriptorWriters[setId] = std::make_shared<Vk::LveDescriptorWriter>(*descriptorSetLayouts[setId], descriptorPool);
        }
        descriptorWriters[setId]->writeImage(bindingId, &imageInfo);
    }

    void SimpleRenderSystem::finishWriteDescriptor()
    {
        assert(descriptorSetLayouts.size() != 0);
        assert(descriptorWriters.size() == descriptorSetLayouts.size());
        descriptorSets.resize(descriptorWriters.size());
        for(int i=0; i<descriptorWriters.size(); i++)
        {
            assert(descriptorWriters[i] != nullptr);
            descriptorWriters[i]->build(descriptorSets[i]);
        }
    }

    void SimpleRenderSystem::bindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
    {
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr
        );
    }


};