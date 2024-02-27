#include "first_app.hpp"

#include "lve_buffer.hpp"
#include "lve_camera.hpp"
#include "simple_render_system.hpp"
#include "keyboard_movement_controller.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <chrono>
#include <stdexcept>
#include <array>

namespace lve
{
    struct GlobalUbo
    {
        alignas(16) glm::mat4 projectionView{1.0f};
        alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{1.0f, -3.0f, -1.0f});
    };

    FirstApp::FirstApp()
    {
        // descriptor pool
        // TODO: add a pool manager to automatically create and free descriptor pools
        globalPool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT) // can create 2 descriptor sets
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT) // have 2 uniform buffer descriptor in total
            .build();

        loadGameObjects();
    }

    FirstApp::~FirstApp()
    {
        globalPool = nullptr;
    }

    void FirstApp::run()
    {
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i<uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  
            );
            uboBuffers[i]->map();
        }

        // this set layout should be matched with shader reflection
        // used in pipeline creation, telling shader bindings
        auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT) // we want one uniform buffer at the binding 0 of vertex shader
            .build();

        // we have 2 descritor sets, whose number is equivelent with resources(buffers and textures)
        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i<globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem simpleRenderSystem(
            lveDevice, 
            lveRenderer.getSwapChainRenderPass(), 
            globalSetLayout->getDescriptorSetLayout());
        LveCamera camera{};
        camera.setViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.0f, 0.0f, 2.5f});

        auto viewerObject = LveGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while(!lveWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            float fps = 1.0f / frameTime;
            std::string title = std::string("hello vulkan!   FPS: ") + std::to_string(fps);
            glfwSetWindowTitle(lveWindow.getGLFWwindow(), title.c_str());

            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);

            if(auto commandBuffer = lveRenderer.beginFrame())
            {
                int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo frameInfo
                {
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex]
                };

                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }

        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    void FirstApp::loadGameObjects()
    {
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "./assets/models/flat_vase.obj");

        auto gameObj = LveGameObject::createGameObject();
        gameObj.model = lveModel;
        gameObj.transform.translation = {0.0f, 0.5f, 2.5f};
        gameObj.transform.scale = glm::vec3{3.0f, 2.0f, 3.0f};
        gameObjects.push_back(std::move(gameObj));
    } 
};