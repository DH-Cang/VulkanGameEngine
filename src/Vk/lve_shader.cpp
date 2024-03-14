#include "lve_shader.hpp"

#include "ThirdParty/utility.hpp"

// std
#include <cassert>
#include <iostream>

namespace Vk
{
    

    LveShader::LveShader(
        LveDevice& device, 
        std::unordered_map<std::string, SetAndBinding>& descriptorSignature, 
        std::vector<std::unique_ptr<LveDescriptorSetLayout>>& descriptorSetLayouts, 
        const std::string& fragShaderPath):
        lveDevice(device), descriptorSignature(descriptorSignature), descriptorSetLayouts(descriptorSetLayouts)
    {
        auto shaderCode = Util::readFile(fragShaderPath);
        createShaderModule(shaderCode, &shaderModule);

        std::vector<ReflectSetLayoutData> reflectionData;
        ShaderReflection(shaderCode, reflectionData); // TODO: only check frag shader

        // according to reflection create descriptor set layout
        for(int i=0; i<reflectionData.size(); i++)
        {
            ReflectSetLayoutData& setLayout = reflectionData[i];
            auto setId = setLayout.set_number;
            while(setId >= descriptorSetLayouts.size())
            {
                descriptorSetLayouts.push_back(nullptr);
            }
            descriptorSetLayouts[setId] = std::make_unique<LveDescriptorSetLayout>(
                device, setLayout.create_info
            );
        }
    }

    LveShader::~LveShader()
    { 
        spvReflectDestroyShaderModule(&module);
        vkDestroyShaderModule(lveDevice.device(), shaderModule, nullptr);
    }

    void LveShader::ShaderReflection(const std::vector<char>& shaderCode, std::vector<ReflectSetLayoutData>& outReflectionData)
    {
        // reflection
        SpvReflectResult result = spvReflectCreateShaderModule(shaderCode.size(), shaderCode.data(), &module);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        uint32_t count = 0;
        result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        reflectDescriptorSets.resize(count);
        result = spvReflectEnumerateDescriptorSets(&module, &count, reflectDescriptorSets.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        // Demonstrates how to generate all necessary data structures to create a
        // VkDescriptorSetLayout for each descriptor set in this shader.
        outReflectionData.resize(count);
        for (size_t i_set = 0; i_set < reflectDescriptorSets.size(); ++i_set) {
            const SpvReflectDescriptorSet& refl_set = *(reflectDescriptorSets[i_set]);
            ReflectSetLayoutData& layout = outReflectionData[i_set];
            layout.bindings.resize(refl_set.binding_count);
            for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) {
                const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[i_binding]);
                VkDescriptorSetLayoutBinding& layout_binding = layout.bindings[i_binding];
                layout_binding.binding = refl_binding.binding;
                layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);
                layout_binding.descriptorCount = 1;
                for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
                    layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
                }
                layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(module.shader_stage);
            }
            layout.set_number = refl_set.set;
            layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout.create_info.bindingCount = refl_set.binding_count;
            layout.create_info.pBindings = layout.bindings.data();
        }
        // Nothing further is done with set_layouts in this sample; in a real
        // application they would be merged with similar structures from other shader
        // stages and/or pipelines to create a VkPipelineLayout.

        //PrintReflectionInfo(module, reflectDescriptorSets);

        // for each set
        for(size_t setId = 0; setId < reflectDescriptorSets.size(); setId++)
        {
            const SpvReflectDescriptorSet& refl_set = *(reflectDescriptorSets[setId]);
            // for each binding
            for (uint32_t bindingId = 0; bindingId < refl_set.binding_count; ++bindingId)
            {
                // record set id and bindingid
                const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[bindingId]);
                std::string name{refl_binding.name};
                SetAndBinding setAndBinding{
                    static_cast<uint32_t>(refl_set.set),
                    refl_binding.binding
                };
                assert(descriptorSignature.find(name) == descriptorSignature.end());
                descriptorSignature[name] = setAndBinding;
            }
        }
    }

    void LveShader::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if(vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("faile to create shader module");
        }
    }
}