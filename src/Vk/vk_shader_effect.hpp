#pragma once

#include "vk_descriptor.hpp"

// std
#include <vector>

namespace Vk
{
    class ShaderEffect
    {
    public:
        ShaderEffect(
            VkDevice device, 
            DescriptorLayoutCache& layoutCache, 
            const std::string& vertShaderPath, 
            const std::string& fragShaderPath);
        ~ShaderEffect();
        ShaderEffect(const ShaderEffect&) = delete;
        ShaderEffect& operator=(const ShaderEffect&) = delete;

        struct SetAndBinding
        {
            uint32_t setId;
            uint32_t bindingId;
            VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
            VkShaderStageFlags stageFlags = 0;
        };

        SetAndBinding getSetAndBinding(const std::string& name) const { return descriptorSignature.find(name)->second; }
        VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
        VkDescriptorSetLayout getSetLayout(int setId) const { return setLayouts[setId]; }
        VkShaderModule getVertShaderModule() const { return vertShader; }
        VkShaderModule getFragShaderModule() const { return fragShader; }

        void printDescriptorSignatures() const
        {
            for(auto& iter : descriptorSignature )
            {
                printf("%s, set %d, binding %d\n", iter.first.c_str(), iter.second.setId, iter.second.bindingId);
            }
        }

    private:
        VkPipelineLayout pipelineLayout;

        std::vector<VkDescriptorSetLayout> setLayouts;
        std::unordered_map<std::string, SetAndBinding> descriptorSignature; // shader reflection data goes into this obj

        constexpr static uint32_t MAX_SET_NUMBER = 10;
        constexpr static uint32_t MAX_BINDING_NUMBER = 10;
        struct ReflectSetLayoutData
        {
            uint32_t setID;
            VkDescriptorSetLayoutCreateInfo create_info{};
            std::vector<VkDescriptorSetLayoutBinding> bindings;
        };
        VkShaderModule vertShader;
        VkShaderModule fragShader;
        std::vector<ReflectSetLayoutData> reflectionData; 

        VkDevice device;
        DescriptorLayoutCache& layoutCache;
        
        
        void loadShaderFromFile(const std::string& vertShaderPath, const std::string& fragShaderPath);
        void getShaderReflection(const std::vector<char>& shaderCode);
        void createDescriptorSetLayouts();
        void createPipelineLayout();
    };
}