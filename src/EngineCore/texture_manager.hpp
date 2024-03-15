#pragma once

#include "Vk/lve_texture.hpp"

// std
#include <unordered_map>
#include <memory>
#include <string>

namespace EngineCore
{
    class TextureManager
    {
    public:
        TextureManager(Vk::LveDevice& device): device(device) {}
        ~TextureManager() = default;
        TextureManager(const TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;

        Vk::LveTexture* addTexture(const std::string& filePath);
        Vk::LveTexture* getTexture(const std::string& filePath);

    private:
        Vk::LveDevice& device;

        std::unordered_map<std::string, std::unique_ptr<Vk::LveTexture>> textureRepo;
    };
}