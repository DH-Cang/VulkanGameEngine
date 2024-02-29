#pragma once

#include "Vk/lve_window.hpp"
#include "Vk/lve_device.hpp"
#include "Vk/lve_game_object.hpp"
#include "Vk/lve_renderer.hpp"
#include "Vk/lve_descriptors.hpp"
#include "Vk/lve_texture.hpp"

// std
#include <memory>

namespace lve {

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

        LveWindow lveWindow{WIDTH, HEIGHT, "hello vulkan"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};

        std::unique_ptr<LveDescriptorPool> globalPool{};
        LveGameObject::Map gameObjects;

        std::unique_ptr<LveTexture> tempTexture;
    };

}