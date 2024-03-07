#pragma once

#include <vulkan/vulkan.h>
#include "lve_buffer.hpp"
#include "lve_descriptors.hpp"

#include "ThirdParty/spirv_reflect.h"

// std
#include <vector>
#include <memory>
#include <unordered_map>

namespace Vk
{
    // handles vertexshader and fragmentshader
    class LveShader
    {
    public:
        LveShader(LveDevice& device, LveDescriptorPool& descriptorPool, const std::string& vertShaderPath, const std::string& fragShaderPath);
        ~LveShader(){ spvReflectDestroyShaderModule(&module); }
        LveShader(const LveShader&) = delete;
        LveShader& operator=(const LveShader&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout(int setIndex) const { return descriptorSetLayouts[setIndex]->getDescriptorSetLayout(); }

        void WriteDescriptor(const std::string& name, VkDescriptorBufferInfo bufferInfo);
        void WriteDescriptor(const std::string& name, VkDescriptorImageInfo imageInfo);
        void FinishWriteDescriptor();
        void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

    private:

        struct DescriptorSetLayoutData
        {
            uint32_t set_number;
            VkDescriptorSetLayoutCreateInfo create_info;
            std::vector<VkDescriptorSetLayoutBinding> bindings;
        };

        struct SetAndBinding
        {
            uint32_t setId;
            uint32_t bindingId;
        };

        SpvReflectShaderModule module = {};
        std::vector<SpvReflectDescriptorSet*> reflectDescriptorSets;


        std::unordered_map<std::string, SetAndBinding> descriptorSignature;
        std::vector<std::shared_ptr<LveDescriptorWriter>> descriptorWriters;
        std::vector<VkDescriptorSet> descriptorSets;
        std::vector<std::unique_ptr<LveDescriptorSetLayout>> descriptorSetLayouts;

        LveDescriptorPool& lveDescriptorPool;

        void ShaderReflection(const std::string& shaderFilePath, std::vector<DescriptorSetLayoutData>& outReflectionData);

    };
}