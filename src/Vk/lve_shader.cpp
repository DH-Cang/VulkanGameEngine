#include "lve_shader.hpp"

#include "lve_swap_chain.hpp"

namespace Vk
{
    LveShader::LveShader(std::vector<LveDescriptorSetLayout*> descriptorSetLayouts):
        descriptorSetLayouts(descriptorSetLayouts) {}

    void LveShader::createBufferAndImage(LveDevice& device, VkDeviceSize size)
    {
        perFrameUniformBuffers.resize(Vk::LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i<perFrameUniformBuffers.size(); i++)
        {
            perFrameUniformBuffers[i] = std::make_unique<Vk::LveBuffer>(
                device,
                size,
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
            perFrameUniformBuffers[i]->map();
        }
    }

    void LveShader::createDescriptorSets(LveDescriptorPool& descriptorPool, VkDescriptorImageInfo temp_texture_info)
    {
        // allocate and write per frame descriptor set
        perFrameDescriptorSets.resize(Vk::LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i<perFrameDescriptorSets.size(); i++)
        {
            auto perFrameBufferInfo = perFrameUniformBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*descriptorSetLayouts[0], descriptorPool)
                .writeBuffer(0, &perFrameBufferInfo)
                .build(perFrameDescriptorSets[i]);
        }

        // allocate and write per object descriptor set
        perObjectDescriptorSets.resize(Vk::LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i<perObjectDescriptorSets.size(); i++)
        {
            //auto perObjectBufferInfo = perObjectUniformBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*descriptorSetLayouts[1], descriptorPool)
                .writeImage(0, &temp_texture_info)
                .build(perObjectDescriptorSets[i]);
        }
    }

    void LveShader::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int frameIndex)
    {
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &perFrameDescriptorSets[frameIndex],
            0,
            nullptr
        );

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            1,
            1,
            &perObjectDescriptorSets[frameIndex],
            0,
            nullptr
        );
    }

    
}