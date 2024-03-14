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
        getShaderReflection(vertShaderCode);

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
        getShaderReflection(fragShaderCode);

    }

    void ShaderEffect::getShaderReflection(const std::vector<char>& shaderCode)
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

        // for each set
        reflectionData.reserve(MAX_SET_NUMBER);
        for (size_t i = 0; i < reflectDescriptorSets.size(); ++i)
        {
            const SpvReflectDescriptorSet& refl_set = *(reflectDescriptorSets[i]);
            auto setID = refl_set.set;
            assert(setID < MAX_SET_NUMBER && "shader's total set number exceed max num");
            if(setID >= reflectionData.size())
                reflectionData.resize(setID + 1);
            ReflectSetLayoutData& out_layout = reflectionData[setID];
            
            // for each binding
            for (uint32_t j = 0; j < refl_set.binding_count; ++j)
            {
                // populate create info
                const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[j]);
                auto bindingID = refl_binding.binding;
                assert(bindingID < MAX_BINDING_NUMBER && "shader's binding number exceed max num");
                if(bindingID >= out_layout.bindings.size())
                    out_layout.bindings.resize(bindingID + 1);
                VkDescriptorSetLayoutBinding& out_binding = out_layout.bindings[bindingID];
                out_binding.binding = bindingID;
                out_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);
                out_binding.descriptorCount = 1;
                for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
                    out_binding.descriptorCount *= refl_binding.array.dims[i_dim];
                }
                out_binding.stageFlags |= static_cast<VkShaderStageFlagBits>(module.shader_stage);

                // populate signature
                std::string name{refl_binding.name};
                auto key_value = descriptorSignature.find(name);
                if( key_value != descriptorSignature.end())
                {
                    assert(
                        descriptorSignature[name].setId == setID &&
                        descriptorSignature[name].bindingId == bindingID &&
                        descriptorSignature[name].type == static_cast<VkDescriptorType>(refl_binding.descriptor_type) &&
                        "error in set and binding type");

                    descriptorSignature[name].stageFlags |= out_binding.stageFlags;
                }
                else
                {
                    SetAndBinding setAndBinding{
                    static_cast<uint32_t>(setID),
                    bindingID,
                    static_cast<VkDescriptorType>(refl_binding.descriptor_type),
                    out_binding.stageFlags
                    };
                    descriptorSignature[name] = setAndBinding;
                }
            }

            out_layout.setID = setID;
            out_layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            out_layout.create_info.bindingCount = out_layout.bindings.size();
            out_layout.create_info.pBindings = out_layout.bindings.data();
        }
        // Nothing further is done with set_layouts in this sample; in a real
        // application they would be merged with similar structures from other shader
        // stages and/or pipelines to create a VkPipelineLayout.

        // PrintReflectionInfo(module, reflectDescriptorSets);

        spvReflectDestroyShaderModule(&module);
    }

    void ShaderEffect::createDescriptorSetLayouts()
    {
        // according to reflection create descriptor set layout
        for(int i = 0; i < reflectionData.size(); i++)
        {
            ReflectSetLayoutData& setLayout = reflectionData[i];
            auto setId = setLayout.setID;
            while(setId >= setLayouts.size())
            {
                setLayouts.push_back(nullptr);
            }
            setLayouts[setId] = layoutCache.create_descriptor_layout(&setLayout.create_info);
        }
    }

    void ShaderEffect::createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        pipelineLayoutInfo.pSetLayouts = setLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }
}