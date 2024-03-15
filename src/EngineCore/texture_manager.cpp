#include "texture_manager.hpp"

// std
#include <cassert>

namespace EngineCore
{
    Vk::LveTexture* TextureManager::addTexture(const std::string& filePath)
    {
        if(textureRepo.find(filePath) == textureRepo.end())
        {
            textureRepo[filePath] = Vk::LveTexture::createTextureFromFile(device, filePath);
        }

        return textureRepo[filePath].get();
    }

    Vk::LveTexture* TextureManager::getTexture(const std::string& filePath)
    {
        assert(textureRepo.find(filePath) != textureRepo.end());
        return textureRepo[filePath].get();
    }
}