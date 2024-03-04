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
    // TODO: add a pool manager to automatically create and free descriptor pools
    globalPool = Vk::LveDescriptorPool::Builder(lveDevice)
        .setMaxSets(Vk::LveSwapChain::MAX_FRAMES_IN_FLIGHT) // can create 2 descriptor sets
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Vk::LveSwapChain::MAX_FRAMES_IN_FLIGHT) // have 2 uniform buffer descriptor in total
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Vk::LveSwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();

    loadGameObjects();
}

FirstApp::~FirstApp()
{
    globalPool = nullptr;
}

void FirstApp::run()
{
    std::vector<std::unique_ptr<Vk::LveBuffer>> uboBuffers(Vk::LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i=0; i<uboBuffers.size(); i++)
    {
        uboBuffers[i] = std::make_unique<Vk::LveBuffer>(
            lveDevice,
            sizeof(EngineCore::GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        uboBuffers[i]->map();
    }

    // this set layout should be matched with shader reflection
    // used in pipeline creation, telling shader bindings
    auto globalSetLayout = Vk::LveDescriptorSetLayout::Builder(lveDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT) // we want one uniform buffer at the binding 0 of vertex shader and frag shader
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    // we have 2 descritor sets, whose number is equivelent with resources(buffers and textures)
    std::vector<VkDescriptorSet> globalDescriptorSets(Vk::LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i=0; i<globalDescriptorSets.size(); i++)
    {
        auto uboInfo = uboBuffers[i]->descriptorInfo();
        auto textureInfo = tempTexture->getDescriptorImageInfo();
        Vk::LveDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &uboInfo)
            .writeImage(1, &textureInfo)
            .build(globalDescriptorSets[i]);
    }

    EngineSystem::SimpleRenderSystem simpleRenderSystem{
        lveDevice, 
        lveRenderer.getSwapChainRenderPass(), 
        globalSetLayout->getDescriptorSetLayout()
    };
    EngineSystem::PointLightSystem pointLightSystem{
        lveDevice, 
        lveRenderer.getSwapChainRenderPass(), 
        globalSetLayout->getDescriptorSetLayout()
    };


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
                globalDescriptorSets[frameIndex],
                gameObjects
            };

            // update
            EngineCore::GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            pointLightSystem.update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

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