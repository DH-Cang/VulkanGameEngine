#pragma once

#include "lve_device.hpp"

// std
#include <string>
#include <memory>

namespace Vk
{
    class LveTexture
    {
    public:

        struct Builder
        {
            int width;
            int height;
            int channels;
            void* data = nullptr;

            void loadTextureFromFile(const std::string& path);
            ~Builder();
        };

        LveTexture(LveDevice& device, const Builder& builder);
        ~LveTexture();

        LveTexture(const LveTexture&) = delete;
        LveTexture& operator=(const LveTexture&) = delete;

        static std::unique_ptr<LveTexture> createTextureFromFile(LveDevice& device, const std::string& filePath);
        VkDescriptorImageInfo getDescriptorImageInfo();

    private:
        // create 2d rgba texture by default
        void createTexture(void* data);
        void createTextureImageView();
        void createTextureSampler();

        LveDevice& lveDevice;

        int width;
        int height;
        int channels;
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;

        VkImageView textureImageView;

        VkSampler textureSampler;
    };
}