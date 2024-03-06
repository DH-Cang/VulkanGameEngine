#pragma once

#include "camera.hpp"
#include "game_object.hpp"

#include "Vk/lve_shader.hpp"

// lib
#include <vulkan/vulkan.h>

namespace EngineCore
{

#define MAX_LIGHTS 10

    struct PointLight
    {
        glm::vec4 position{}; // ignore w
        alignas(16) glm::vec4 color{}; // w is intensity
    };

    struct GlobalUbo
    {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
        glm::mat4 inverseView{1.0f};
        glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f}; // w is light intensity
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera& camera;
        Vk::LveShader& lveShader;
        EngineCore::GameObject::Map& gameObjects;
    };
}