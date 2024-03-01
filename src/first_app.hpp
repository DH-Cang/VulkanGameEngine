#pragma once

#include "Platform/my_window.hpp"
#include "Vk/lve_device.hpp"
#include "Vk/lve_renderer.hpp"
#include "Vk/lve_descriptors.hpp"
#include "Vk/lve_texture.hpp"

#include "EngineCore/game_object.hpp"

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

    std::unique_ptr<Vk::LveDescriptorPool> globalPool{};
    EngineCore::GameObject::Map gameObjects;

    std::unique_ptr<Vk::LveTexture> tempTexture;
};