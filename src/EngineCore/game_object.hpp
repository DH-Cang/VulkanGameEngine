#pragma once

#include "model.hpp"
#include "Vk/vk_descriptor.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

//std
#include <memory>
#include <unordered_map>

namespace EngineCore
{
    struct TransformComponent
    {
        glm::vec3 translation{}; // position offset
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{}; // euler angles: Y-X-Z  (in radians)

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4();
        glm::mat3 normalMatrix();

        VkDescriptorSet descriptorSet;
        std::shared_ptr<Vk::LveBuffer> ubo = nullptr;
    };

    struct PerObjectUboData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
        //alignas(16) glm::vec3 color; // make sure the memory align as 16 bytes, to match the data alignment in shader
    };

    struct PointLightComponent
    {
        float lightIntensity = 1.0f;

        VkDescriptorSet descriptorSet;
        std::shared_ptr<Vk::LveBuffer> ubo = nullptr;
    };

    struct PointLightPerObjectData
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    class GameObject
    {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, GameObject>;

        static GameObject createGameObject(Vk::LveDevice& lveDevice, Vk::DescriptorAllocator& descriptorAllocator, Vk::DescriptorLayoutCache& descriptorLayoutCache )
        {
            static id_t currentId = 0;
            return GameObject{currentId++, lveDevice, descriptorAllocator, descriptorLayoutCache};
        }
        
        static GameObject makePointLight(
            Vk::LveDevice& lveDevice, Vk::DescriptorAllocator& descriptorAllocator, Vk::DescriptorLayoutCache& descriptorLayoutCache,
            float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3{1.0f}
        );

        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = default;

        id_t getId() { return id; }

        glm::vec3 color{};
        void updateSimpleObject();
        void updatePointLightObject();
        VkDescriptorSet getSimpleDescriptor() { return transform.descriptorSet; }
        VkDescriptorSet getPointLightDescriptor() { assert(pointLight); return pointLight->descriptorSet; }

        // optional pointer component
        std::shared_ptr<Model> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

        TransformComponent transform;
    private:
        GameObject(id_t objId, Vk::LveDevice& lveDevice, Vk::DescriptorAllocator& descriptorAllocator, Vk::DescriptorLayoutCache& descriptorLayoutCache);
        id_t id;

    };

}