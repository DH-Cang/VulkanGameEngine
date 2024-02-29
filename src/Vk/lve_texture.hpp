#pragma once

#include "lve_buffer.hpp"

// std
#include <string>

namespace lve
{
    class LveTexture
    {
    public:
        LveTexture(LveDevice& device);
        ~LveTexture();

        LveTexture(const LveTexture&) = delete;
        LveTexture& operator=(const LveTexture&) = delete;

        void createTextureFromFile(std::string& path);

        void createTextureImageView();

        void createTextureSampler();

        VkDescriptorImageInfo getDescriptorImageInfo();

    private:
        LveDevice& lveDevice;

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;

        VkImageView textureImageView;

        VkSampler textureSampler;
    };
}