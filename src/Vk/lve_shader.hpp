#pragma once

#include "lve_buffer.hpp"
#include "lve_descriptors.hpp"

// std
#include <vector>
#include <memory>

namespace Vk
{
    // handles vertexshader and fragmentshader
    class LveShader
    {
    public:
        LveShader(std::vector<LveDescriptorSetLayout*> descriptorSetLayouts);
        ~LveShader() = default;
        LveShader(const LveShader&) = delete;
        LveShader& operator=(const LveShader&) = delete;

        // create fixed buffer and image for now
        void createBufferAndImage(LveDevice& device, VkDeviceSize size);
        void writeToBuffer(void* data, int frameIndex) { perFrameUniformBuffers[frameIndex]->writeToBuffer(data); }

        // allocate descriptor sets from pool
        // populate descriptor sets with buffer info // temp texture info to be deleted
        void createDescriptorSets(LveDescriptorPool& descriptorPool, VkDescriptorImageInfo temp_texture_info);

        // load data to actual memory in gpu
        // bind descriptor sets to render pipeline // temp: set id
        void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int frameIndex);

    public:
        // temp
        // descriptor set layout [0]: per frame set
        // descriptor set layout [1]: per object set
        std::vector<LveDescriptorSetLayout*> descriptorSetLayouts;

        std::vector<std::unique_ptr<LveBuffer>> rarelyChangeUniformBuffers; // size == frame buffer num
        std::vector<std::unique_ptr<LveBuffer>> perFrameUniformBuffers;
        std::vector<std::unique_ptr<LveBuffer>> perObjectUniformBuffers;

        std::vector<VkDescriptorSet> rarelyChangeDescriptorSets; // size == frame buffer num | set 0
        std::vector<VkDescriptorSet> perFrameDescriptorSets; // size == frame buffer num | set 1
        std::vector<VkDescriptorSet> perObjectDescriptorSets; // size == frame buffer num | set 2
    };
}