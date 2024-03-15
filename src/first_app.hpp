#pragma once

#include "Platform/my_window.hpp"
#include "Vk/lve_device.hpp"
#include "Vk/lve_renderer.hpp"

#include "EngineCore/game_object.hpp"
#include "EngineCore/texture_manager.hpp"

// std
#include <memory>

class FirstApp
{
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    FirstApp();
    ~FirstApp();

    FirstApp(const FirstApp&) = delete;
    FirstApp& operator=(const FirstApp&) = delete;

    void run();

private:
    void loadGameObjects();

    Platform::MyWindow myWindow{WIDTH, HEIGHT, "hello vulkan"};
    Vk::LveDevice lveDevice{myWindow};
    Vk::LveRenderer lveRenderer{myWindow, lveDevice};
    EngineCore::TextureManager textureManager{lveDevice};
    Vk::DescriptorAllocator descriptorAllocator{lveDevice.device()};
    Vk::DescriptorLayoutCache descriptorLayoutCache{lveDevice.device()};

    EngineCore::GameObject::Map gameObjects;

};