#pragma once

#include "Vk/lve_buffer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <memory>

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

        std::string ambientTextureName;
        std::string roughnessTextureName;
        std::string metallicTextureName;
        

        // Vk::LvePipeline& pipeline;
        VkDescriptorSet descriptorSet;
        std::shared_ptr<Vk::LveBuffer> ubo = nullptr;
    };
}

