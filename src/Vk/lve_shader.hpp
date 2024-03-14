#pragma once

#include <vulkan/vulkan.h>
#include "lve_buffer.hpp"
#include "lve_descriptors.hpp"

#include "ThirdParty/SpirvReflection/spirv_reflect.h"

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
        struct SetAndBinding
        {
            uint32_t setId;
            uint32_t bindingId;
            VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
        };

        LveShader(
            LveDevice& device, 
            std::unordered_map<std::string, SetAndBinding>& descriptorSignature, 
            std::vector<std::unique_ptr<LveDescriptorSetLayout>>& descriptorSetLayouts, 
            const std::string& fragShaderPath);

        ~LveShader();
        LveShader(const LveShader&) = delete;
        LveShader& operator=(const LveShader&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout(int setIndex) const { return descriptorSetLayouts[setIndex]->getDescriptorSetLayout(); }

        VkShaderModule getShaderModule() const { return shaderModule; }

        struct ReflectSetLayoutData
        {
            uint32_t set_number;
            VkDescriptorSetLayoutCreateInfo create_info;
            std::vector<VkDescriptorSetLayoutBinding> bindings;
        };

        VkShaderModule shaderModule;
    private:
        LveDevice& lveDevice;
        std::unordered_map<std::string, SetAndBinding>& descriptorSignature;
        std::vector<std::unique_ptr<LveDescriptorSetLayout>>& descriptorSetLayouts;

        SpvReflectShaderModule module = {};
        std::vector<SpvReflectDescriptorSet*> reflectDescriptorSets;

        void ShaderReflection(const std::vector<char>& shaderCode, std::vector<ReflectSetLayoutData>& outReflectionData);

        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        
    };
}