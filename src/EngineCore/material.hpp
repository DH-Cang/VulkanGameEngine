#pragma once

#include "Vk/lve_descriptors.hpp"
#include "Vk/lve_pipeline.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <unordered_map>
#include <string>

namespace EngineCore
{
    class Material
    {
    public:
        struct Data
        {
            glm::vec4 ambient; // ignore w
            float blinn_factor = 32.0f;
            // ....        
        } materialData;
        

        // Vk::LvePipeline& pipeline;
        VkDescriptorSet descriptorSet;
        bool is_descriptor_allocated = false;
        std::shared_ptr<Vk::LveBuffer> ubo = nullptr;
    };
}

