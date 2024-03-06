#include "lve_shader.hpp"
#include "ThirdParty/spirv_reflect.h"
#include "ThirdParty/utility.hpp"

// std
#include <cassert>
#include <iostream>

// tmp
#include "lve_swap_chain.hpp"
#include "temp.hpp"

namespace Vk
{
    

    LveShader::LveShader(LveDevice& device, const std::string& vertShaderPath, const std::string& fragShaderPath)
    {
        std::vector<DescriptorSetLayoutData> reflectionData;
        ShaderReflection(fragShaderPath, reflectionData); // TODO: only check frag shader

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

        SpvReflectShaderModule module = {};
        SpvReflectResult result = spvReflectCreateShaderModule(shaderCode.size(), shaderCode.data(), &module);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        uint32_t count = 0;
        result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        std::vector<SpvReflectDescriptorSet*> sets(count);
        result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        // Demonstrates how to generate all necessary data structures to create a
        // VkDescriptorSetLayout for each descriptor set in this shader.
        outReflectionData.resize(count);
        for (size_t i_set = 0; i_set < sets.size(); ++i_set) {
            const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);
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

        // Log the descriptor set contents to stdout
        const char* t = "  ";
        const char* tt = "    ";

        PrintModuleInfo(std::cout, module);
        std::cout << "\n\n";

        std::cout << "Descriptor sets:"
                    << "\n";
        for (size_t index = 0; index < sets.size(); ++index) {
            auto p_set = sets[index];

            // descriptor sets can also be retrieved directly from the module, by set
            // index
            auto p_set2 = spvReflectGetDescriptorSet(&module, p_set->set, &result);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);
            assert(p_set == p_set2);
            (void)p_set2;

            std::cout << t << index << ":"
                    << "\n";
            PrintDescriptorSet(std::cout, *p_set, tt);
            std::cout << "\n\n";
        }

        spvReflectDestroyShaderModule(&module);
    }

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