#include "vk_shader_effect.hpp"

#include "ThirdParty/utility.hpp"
#include "ThirdParty/SpirvReflection/spirv_reflect.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <cassert>
#include <iostream>

namespace Vk
{
    ShaderEffect::ShaderEffect(VkDevice device, DescriptorLayoutCache& layoutCache, const std::string& vertShaderPath, const std::string& fragShaderPath):
        device(device), layoutCache(layoutCache)
    {
        loadShaderFromFile(vertShaderPath, fragShaderPath);
        createDescriptorSetLayouts();
        createPipelineLayout();
    }

    ShaderEffect::~ShaderEffect()
    {
        vkDestroyShaderModule(device, vertShader, nullptr);
        vkDestroyShaderModule(device, fragShader, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }

    void ShaderEffect::loadShaderFromFile(const std::string& vertShaderPath, const std::string& fragShaderPath)
    {
        // load vertex shader
        auto vertShaderCode = Util::readFile(vertShaderPath);
        VkShaderModuleCreateInfo createInfoVert{};
        createInfoVert.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfoVert.codeSize = vertShaderCode.size();
        createInfoVert.pCode = reinterpret_cast<const uint32_t*>(vertShaderCode.data());
        if(vkCreateShaderModule(device, &createInfoVert, nullptr, &vertShader) != VK_SUCCESS)
        {
            throw std::runtime_error("faile to create shader module");
        }
        getShaderReflection(vertShaderCode, vertReflectData);

        // load frag shader
        auto fragShaderCode = Util::readFile(fragShaderPath);
        VkShaderModuleCreateInfo createInfoFrag{};
        createInfoFrag.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfoFrag.codeSize = fragShaderCode.size();
        createInfoFrag.pCode = reinterpret_cast<const uint32_t*>(fragShaderCode.data());
        if(vkCreateShaderModule(device, &createInfoFrag, nullptr, &fragShader) != VK_SUCCESS)
        {
            throw std::runtime_error("faile to create shader module");
        }
        getShaderReflection(fragShaderCode, fragReflectData);

    }

    void ShaderEffect::getShaderReflection(const std::vector<char>& shaderCode, std::vector<ReflectSetLayoutData>& outReflectionData)
    {
        SpvReflectShaderModule module = {};
        // reflection
        SpvReflectResult result = spvReflectCreateShaderModule(shaderCode.size(), shaderCode.data(), &module);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        uint32_t count = 0;
        result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        std::vector<SpvReflectDescriptorSet*> reflectDescriptorSets(count);
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
                    refl_binding.binding,
                    static_cast<VkDescriptorType>(refl_binding.descriptor_type),
                };
                assert(descriptorSignature.find(name) == descriptorSignature.end());
                descriptorSignature[name] = setAndBinding;
            }
        }

        spvReflectDestroyShaderModule(&module);
    }

    void ShaderEffect::createDescriptorSetLayouts()
    {
        // according to reflection create descriptor set layout, vertex shader
        for(int i = 0; i < vertReflectData.size(); i++)
        {
            ReflectSetLayoutData& setLayout = vertReflectData[i];
            auto setId = setLayout.set_number;
            while(setId >= setLayouts.size())
            {
                setLayouts.push_back(nullptr);
            }
            setLayouts[setId] = layoutCache.create_descriptor_layout(&setLayout.create_info);
        }

        // according to reflection create descriptor set layout, vertex shader
        for(int i = 0; i < fragReflectData.size(); i++)
        {
            ReflectSetLayoutData& setLayout = fragReflectData[i];
            auto setId = setLayout.set_number;
            while(setId >= setLayouts.size())
            {
                setLayouts.push_back(nullptr);
            }
            setLayouts[setId] = layoutCache.create_descriptor_layout(&setLayout.create_info);
        }
    }

    // temp
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
        //alignas(16) glm::vec3 color; // make sure the memory align as 16 bytes, to match the data alignment in shader
    };

    void ShaderEffect::createPipelineLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        pipelineLayoutInfo.pSetLayouts = setLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }


}