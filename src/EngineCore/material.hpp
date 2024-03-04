#pragma once

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
        glm::vec3 ambient;
        float blinn_factor = 32.0f;
        // ....        
    };
}

