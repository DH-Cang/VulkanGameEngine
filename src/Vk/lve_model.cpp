#include "lve_model.hpp"

// libs
#include "ThirdParty\utility.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include "ThirdParty\tiny_obj_loader.h"


// std
#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

namespace std
{
    template <>
    struct hash<Vk::LveModel::Vertex>
    {
        size_t operator()(Vk::LveModel::Vertex const& vertex) const
        {
            size_t seed = 0;
            Util::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace Vk
{

    LveModel::LveModel(LveDevice& device, const LveModel::Builder& builder):
        lveDevice(device)
    {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    LveModel::~LveModel()
    {}

    // first copy data from host to staging buffer, and then copy data from staging buffer to device buffer
    void LveModel::createVertexBuffers(const std::vector<Vertex>& vertices)
    {
        vertex_count = static_cast<uint32_t>(vertices.size());
        assert(vertex_count >= 3 && "vertex count must be at least 3");
        uint32_t vertexSize = sizeof(vertices[0]);

        LveBuffer stagingBuffer
        {
            lveDevice,
            vertexSize,
            vertex_count,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data()); 
        // unmap will be handled by buffer cleaning up

        vertexBuffer = std::make_unique<LveBuffer>(
            lveDevice,
            vertexSize,
            vertex_count,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        lveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), stagingBuffer.getBufferSize());
    }

    void LveModel::createIndexBuffers(const std::vector<uint32_t>& indices)
    {
        index_count = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = index_count > 0;
        if(hasIndexBuffer == false)
        {
            return;
        }

        uint32_t indexSize = sizeof(indices[0]);

        LveBuffer stagingBuffer
        {
            lveDevice,
            indexSize,
            index_count,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices.data());
        // unmap will be handled by buffer cleaning up

        indexBuffer = std::make_unique<LveBuffer>(
            lveDevice,
            indexSize,
            index_count,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        
        lveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), stagingBuffer.getBufferSize());
    }

    void LveModel::draw(VkCommandBuffer commandBuffer)
    {
        if(hasIndexBuffer)
        {
            vkCmdDrawIndexed(commandBuffer, index_count, 1, 0, 0, 0);
        }
        else 
        {
            vkCmdDraw(commandBuffer, vertex_count, 1, 0, 0);    
        }
    }

    void LveModel::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        if(hasIndexBuffer)
        {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescription()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
    }
}