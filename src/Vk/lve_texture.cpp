#include "lve_texture.hpp"

// lib
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty\stb_image.h"

// std
#include <stdexcept>
#include <iostream>

namespace lve
{
    LveTexture::LveTexture(LveDevice& device): lveDevice(device)
    {}

    LveTexture::~LveTexture()
    {
        vkDestroySampler(lveDevice.device(), textureSampler, nullptr);

        vkDestroyImageView(lveDevice.device(), textureImageView, nullptr);

        vkDestroyImage(lveDevice.device(), textureImage, nullptr);
        vkFreeMemory(lveDevice.device(), textureImageMemory, nullptr);
    }

    void LveTexture::createTextureFromFile(std::string& path)
    {
        // load texture using stbi_load()
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4; // TODO: why count?
        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        // create staging buffer
        LveBuffer stagingBuffer
        {
            lveDevice,
            imageSize,
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        // copy data to staging buffer
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*) pixels);

        // clean up original pixel data
        stbi_image_free(pixels);

        // image create info
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(texWidth);
        imageInfo.extent.height = static_cast<uint32_t>(texHeight);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        lveDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        lveDevice.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        lveDevice.copyBufferToImage(stagingBuffer.getBuffer(), textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1);
        lveDevice.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void LveTexture::createTextureImageView()
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = textureImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(lveDevice.device(), &viewInfo, nullptr, &textureImageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void LveTexture::createTextureSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = lveDevice.properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(lveDevice.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
        throw std::runtime_error("failed to create texture sampler!");
        }
    }
    

}