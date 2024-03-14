#pragma once

#include "vk_descriptor.hpp"

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

        struct ReflectSetLayoutData
        {
            uint32_t set_number;
            VkDescriptorSetLayoutCreateInfo create_info;
            std::vector<VkDescriptorSetLayoutBinding> bindings;
        };
        // note that there must be no repeated set/binding/name in shader
        VkShaderModule vertShader;
        std::vector<ReflectSetLayoutData> vertReflectData; // size == number of sets in vertex shader
        VkShaderModule fragShader;
        std::vector<ReflectSetLayoutData> fragReflectData; // size == number of sets in frag shader

        VkDevice device;
        DescriptorLayoutCache& layoutCache;
        

        void loadShaderFromFile(const std::string& vertShaderPath, const std::string& fragShaderPath);
        void getShaderReflection(const std::vector<char>& shaderCode, std::vector<ReflectSetLayoutData>& outReflectionData);
        void createDescriptorSetLayouts();
        void createPipelineLayout();
    };
}