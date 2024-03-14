#include "first_app.hpp"

#include "Vk/lve_buffer.hpp"

#include "EngineCore/camera.hpp"
#include "EngineCore/keyboard_movement_controller.hpp"
#include "EngineCore/model.hpp"

#include "EngineSystems/simple_render_system.hpp"
#include "EngineSystems/point_light_system.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <chrono>


FirstApp::FirstApp()
{
    // descriptor pool
    globalPool = Vk::LveDescriptorPool::Builder(lveDevice)
        .setMaxSets(1000) // can create 2 descriptor sets
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000) // have 2 uniform buffer descriptor in total
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 50)
        .build();

    loadGameObjects();
}

FirstApp::~FirstApp()
{
    globalPool = nullptr;
}

void FirstApp::run()
{
    // create ubo
    std::unique_ptr<Vk::LveBuffer> globalUbo;
    globalUbo = std::make_unique<Vk::LveBuffer>(
        lveDevice,
        sizeof(EngineCore::GlobalUbo),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    globalUbo->map();

    Vk::DescriptorLayoutCache descriptorLayoutCache{lveDevice.device()};

    EngineSystem::SimpleRenderSystem simpleRenderSystem{
        lveDevice, 
        descriptorLayoutCache,
        lveRenderer.getSwapChainRenderPass()
    };
    EngineSystem::PointLightSystem pointLightSystem{
        lveDevice, 
        lveRenderer.getSwapChainRenderPass()
    };

    simpleRenderSystem.createDescriptorSetPerFrame("ubo", globalUbo->descriptorInfo(), VK_SHADER_STAGE_FRAGMENT_BIT);
    simpleRenderSystem.createDescriptorSetPerFrame("texSampler", tempTexture->getDescriptorImageInfo(), VK_SHADER_STAGE_FRAGMENT_BIT);
    simpleRenderSystem.createDescriptorSetPerFrame("uboVert", globalUbo->descriptorInfo(), VK_SHADER_STAGE_VERTEX_BIT);
    simpleRenderSystem.finishCreateDescriptorSetPerFrame();

    pointLightSystem.writeDescriptorToSets("ubo", globalUbo->descriptorInfo(), *globalPool);
    pointLightSystem.writeDescriptorToSets("texSampler", tempTexture->getDescriptorImageInfo(), *globalPool);
    pointLightSystem.writeDescriptorToSets("uboVert", globalUbo->descriptorInfo(), *globalPool);
    pointLightSystem.writeDescriptorToSets("texSamplerVert", tempTexture->getDescriptorImageInfo(), *globalPool);
    pointLightSystem.finishWriteDescriptor();

    //=================================== update camera object .etc =================================

    EngineCore::Camera camera{};
    camera.setViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.0f, 0.0f, 2.5f});

    auto viewerObject = EngineCore::GameObject::createGameObject();
    viewerObject.transform.translation.z = -2.5f;
    EngineCore::KeyboardMovementController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();

    while(!myWindow.shouldClose())
    {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        float fps = 1.0f / frameTime;
        std::string title = std::string("hello vulkan!   FPS: ") + std::to_string(fps);
        glfwSetWindowTitle(myWindow.getGLFWwindow(), title.c_str());

        cameraController.moveInPlaneXZ(myWindow.getGLFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = lveRenderer.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 1000.0f);

        if(auto commandBuffer = lveRenderer.beginFrame())
        {
            int frameIndex = lveRenderer.getFrameIndex();
            EngineCore::FrameInfo frameInfo
            {
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                gameObjects
            };

            // update
            EngineCore::GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            pointLightSystem.update(frameInfo, ubo);
            globalUbo->writeToBuffer(&ubo);

            // render
            lveRenderer.beginSwapChainRenderPass(commandBuffer);

            // order matters
            simpleRenderSystem.renderGameObjects(frameInfo);
            pointLightSystem.render(frameInfo);

            lveRenderer.endSwapChainRenderPass(commandBuffer);
            lveRenderer.endFrame();
        }

    }

    vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects()
{
    tempTexture = Vk::LveTexture::createTextureFromFile(lveDevice, ".\\assets\\textures\\70591182.jpg");

    std::shared_ptr<EngineCore::Model> model = EngineCore::Model::createModelFromFile(lveDevice, "./assets/models/flat_vase.obj", "./assets/models/");
    auto flatVase = EngineCore::GameObject::createGameObject();
    flatVase.model = model;
    flatVase.transform.translation = {-0.5f, 0.5f, 0.0f};
    flatVase.transform.scale = glm::vec3{3.0f, 2.0f, 3.0f};
    gameObjects.emplace(flatVase.getId(), std::move(flatVase));

    model = EngineCore::Model::createModelFromFile(lveDevice, "./assets/models/smooth_vase.obj", "./assets/models/");
    auto smoothVase = EngineCore::GameObject::createGameObject();
    smoothVase.model = model;
    smoothVase.transform.translation = {0.5f, 0.5f, 0.0f};
    smoothVase.transform.scale = glm::vec3{3.0f, 2.0f, 3.0f};
    gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

    model = EngineCore::Model::createModelFromFile(lveDevice, "./assets/models/quad.obj", "./assets/models/");
    auto floor = EngineCore::GameObject::createGameObject();
    floor.model = model;
    floor.transform.translation = {0.0f, 0.5f, 0.0f};
    floor.transform.scale = glm::vec3{3.0f, 1.0f, 3.0f};
    gameObjects.emplace(floor.getId(), std::move(floor));

    model = EngineCore::Model::createModelFromFile(lveDevice, "./assets/models/cube.obj", "./assets/models/");
    auto cube = EngineCore::GameObject::createGameObject();
    cube.model = model;
    cube.transform.translation = {0.0f, 0.0f, -1.0f};
    cube.transform.scale = glm::vec3{0.25f, 0.25f, 0.25f};
    gameObjects.emplace(cube.getId(), std::move(cube));


    std::vector<glm::vec3> lightColors{
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
        {1.f, 1.f, 1.f}  //
    };

    for(int i=0; i < lightColors.size(); i++)
    {
        auto pointLight = EngineCore::GameObject::makePointLight(0.2f);
        pointLight.color = lightColors[i];
        auto rotateLight = glm::rotate(
            glm::mat4(1.0f),
            (i * glm::two_pi<float>()) / lightColors.size(),
            {0.0f, -1.0f, 0.0f}
        );
        pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f));
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }   


} 