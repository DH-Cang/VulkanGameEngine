#include "game_object.hpp"

namespace EngineCore
{
    glm::mat4 TransformComponent::mat4()
    {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        return glm::mat4{
            {
                scale.x * (c1 * c3 + s1 * s2 * s3),
                scale.x * (c2 * s3),
                scale.x * (c1 * s2 * s3 - c3 * s1),
                0.0f,
            },
            {
                scale.y * (c3 * s1 * s2 - c1 * s3),
                scale.y * (c2 * c3),
                scale.y * (c1 * c3 * s2 + s1 * s3),
                0.0f,
            },
            {
                scale.z * (c2 * s1),
                scale.z * (-s2),
                scale.z * (c1 * c2),
                0.0f,
            },
            {translation.x, translation.y, translation.z, 1.0f}};
    }

    glm::mat3 TransformComponent::normalMatrix()
    {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const glm::vec3 invScale = 1.0f / scale;

        return glm::mat3{
            {
                invScale.x * (c1 * c3 + s1 * s2 * s3),
                invScale.x * (c2 * s3),
                invScale.x * (c1 * s2 * s3 - c3 * s1),
            },
            {
                invScale.y * (c3 * s1 * s2 - c1 * s3),
                invScale.y * (c2 * c3),
                invScale.y * (c1 * c3 * s2 + s1 * s3),
            },
            {
                invScale.z * (c2 * s1),
                invScale.z * (-s2),
                invScale.z * (c1 * c2),
            }};
    }

    GameObject::GameObject(id_t objId, Vk::LveDevice& lveDevice, Vk::DescriptorAllocator& descriptorAllocator, Vk::DescriptorLayoutCache& descriptorLayoutCache) : 
        id(objId)
    {
        transform.ubo = std::make_shared<Vk::LveBuffer>(
            lveDevice,
            sizeof(PerObjectUboData),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        transform.ubo->map();

        auto descriptorInfo = transform.ubo->descriptorInfo();
        Vk::DescriptorBuilder builder(descriptorLayoutCache, descriptorAllocator);
        builder.bind_buffer(0, &descriptorInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).build(transform.descriptorSet);
    }   

    void GameObject::updateSimpleObject()
    {
        PerObjectUboData perObjectUboData
        {
            transform.mat4(),
            transform.normalMatrix()
        };
        transform.ubo->writeToBuffer(&perObjectUboData);
    }

    void GameObject::updatePointLightObject()
    {
        PointLightPerObjectData perObjectUboData
        {
            glm::vec4(transform.translation, 1.0f),
            glm::vec4(color, pointLight->lightIntensity),
            transform.scale.x
        };
        pointLight->ubo->writeToBuffer(&perObjectUboData);
    }

    GameObject GameObject::makePointLight(
        Vk::LveDevice& lveDevice, Vk::DescriptorAllocator& descriptorAllocator, Vk::DescriptorLayoutCache& descriptorLayoutCache,
        float intensity, float radius, glm::vec3 color)
    {
        GameObject gameObj = GameObject::createGameObject(lveDevice, descriptorAllocator, descriptorLayoutCache);
        gameObj.color = color;
        gameObj.transform.scale.x = radius;
        gameObj.pointLight = std::make_unique<PointLightComponent>();
        gameObj.pointLight->lightIntensity = intensity;

        gameObj.pointLight->ubo = std::make_shared<Vk::LveBuffer>(
            lveDevice,
            sizeof(PointLightPerObjectData),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        gameObj.pointLight->ubo->map();

        auto descriptorInfo = gameObj.pointLight->ubo->descriptorInfo();
        Vk::DescriptorBuilder builder(descriptorLayoutCache, descriptorAllocator);
        builder.bind_buffer(0, &descriptorInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT).build(gameObj.pointLight->descriptorSet);

        return gameObj;
    }

}
