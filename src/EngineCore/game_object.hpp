#pragma once

#include "model.hpp"

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
    };

    struct PointLightComponent
    {
        float lightIntensity = 1.0f;
    };

    class GameObject
    {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, GameObject>;

        static GameObject createGameObject()
        {
            static id_t currentId = 0;
            return GameObject{currentId++};
        }
        
        static GameObject makePointLight(
            float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3{1.0f}
        );

        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = default;
        //GameObject& operator=(const GameObject&&) = default;

        id_t getId() { return id; }

        glm::vec3 color{};
        TransformComponent transform;

        // optional pointer component
        std::shared_ptr<Model> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

    private:
        GameObject(id_t objId) : id(objId) {}
        id_t id;

    };

}