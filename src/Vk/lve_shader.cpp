#include "lve_shader.hpp"

#include "ThirdParty/utility.hpp"

// std
#include <cassert>
#include <iostream>

// tmp
#include "lve_swap_chain.hpp"
#include "temp.hpp"

namespace Vk
{
    

    LveShader::LveShader(LveDevice& device, LveDescriptorPool& descriptorPool, const std::string& vertShaderPath, const std::string& fragShaderPath):
        lveDescriptorPool(descriptorPool)
    {
        std::vector<DescriptorSetLayoutData> reflectionData;
        ShaderReflection(fragShaderPath, reflectionData); // TODO: only check frag shader

        descriptorWriters.resize(reflectionData.size(), nullptr);
        descriptorSetLayouts.resize(reflectionData.size());
        // for each descriptor layout
        for(int i=0; i<reflectionData.size(); i++)
        {
            DescriptorSetLayoutData& setLayout = reflectionData[i];
            descriptorSetLayouts[i] = std::make_unique<LveDescriptorSetLayout>(
                device, setLayout.create_info
            );
        }
    }

    void LveShader::ShaderReflection(const std::string& shaderFilePath, std::vector<DescriptorSetLayoutData>& outReflectionData)
    {
        // reflection
        auto shaderCode = Util::readFile(shaderFilePath);

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
            DescriptorSetLayoutData& layout = outReflectionData[i_set];
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

                // TODO: temp stage flag
                layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            layout.set_number = refl_set.set;
            layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout.create_info.bindingCount = refl_set.binding_count;
            layout.create_info.pBindings = layout.bindings.data();
        }
        // Nothing further is done with set_layouts in this sample; in a real
        // application they would be merged with similar structures from other shader
        // stages and/or pipelines to create a VkPipelineLayout.

        PrintReflectionInfo(module, reflectDescriptorSets);

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
                    static_cast<uint32_t>(setId),
                    refl_binding.binding
                };
                assert(descriptorSignature.find(name) == descriptorSignature.end());
                descriptorSignature[name] = setAndBinding;
            }
        }

        // temp
        for(auto& iter : descriptorSignature )
        {
            printf("%s, set %d, binding %d\n", iter.first.c_str(), iter.second.setId, iter.second.bindingId);
        }
    }


    void LveShader::WriteDescriptor(const std::string& name, VkDescriptorBufferInfo bufferInfo)
    {
        const auto& descriptorRecord = descriptorSignature.find(name);
        assert(descriptorRecord != descriptorSignature.end());
        auto setId = descriptorRecord->second.setId;
        auto bindingId = descriptorRecord->second.bindingId;

        if(descriptorWriters[setId] == nullptr)
        {
            descriptorWriters[setId] = std::make_shared<LveDescriptorWriter>(*descriptorSetLayouts[setId], lveDescriptorPool);
        }
        descriptorWriters[setId]->writeBuffer(bindingId, &bufferInfo);
    }

    void LveShader::WriteDescriptor(const std::string& name, VkDescriptorImageInfo imageInfo)
    {
        const auto& descriptorRecord = descriptorSignature.find(name);
        assert(descriptorRecord != descriptorSignature.end());
        auto setId = descriptorRecord->second.setId;
        auto bindingId = descriptorRecord->second.bindingId;

        if(descriptorWriters[setId] == nullptr)
        {
            descriptorWriters[setId] = std::make_shared<LveDescriptorWriter>(*descriptorSetLayouts[setId], lveDescriptorPool);
        }
        descriptorWriters[setId]->writeImage(bindingId, &imageInfo);
    }

    void LveShader::FinishWriteDescriptor()
    {
        assert(descriptorWriters.size() == descriptorSetLayouts.size());
        descriptorSets.resize(descriptorWriters.size());
        for(int i=0; i<descriptorWriters.size(); i++)
        {
            descriptorWriters[i]->build(descriptorSets[i]);
        }
    }

    void LveShader::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
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

}