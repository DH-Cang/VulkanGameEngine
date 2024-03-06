#pragma once
 
#include "lve_device.hpp"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>
 
namespace Vk {
 
    // helper to build descriptor set layout for pipeline creation
    class LveDescriptorSetLayout
    {
    public:

        class Builder
        {
        public:
            Builder(LveDevice &lveDevice) : lveDevice{lveDevice} {}
    
            Builder &addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<LveDescriptorSetLayout> build() const;
    
        private:
            LveDevice &lveDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };
    
        LveDescriptorSetLayout(
            LveDevice &lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        LveDescriptorSetLayout(
            LveDevice &device, const VkDescriptorSetLayoutCreateInfo& createInfo);
        ~LveDescriptorSetLayout();
        LveDescriptorSetLayout(const LveDescriptorSetLayout &) = delete;
        LveDescriptorSetLayout &operator=(const LveDescriptorSetLayout &) = delete;
        
        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
        
    private:
        LveDevice &lveDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
        
        friend class LveDescriptorWriter;
    };
    
    // helper to build descriptor pool where descriptors are created from
    class LveDescriptorPool
    {
    public:

        class Builder
        {
        public:
            Builder(LveDevice &lveDevice) : lveDevice{lveDevice} {}
        
            Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count); // set the number of each descriptor type
            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count); // how many descriptor sets can be created from this pool
            std::unique_ptr<LveDescriptorPool> build() const;
        
        private:
            LveDevice &lveDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };
    
        LveDescriptorPool(
            LveDevice &lveDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize> &poolSizes);
        ~LveDescriptorPool();
        LveDescriptorPool(const LveDescriptorPool &) = delete;
        LveDescriptorPool &operator=(const LveDescriptorPool &) = delete;
        
        // allocate descripor set from descriptor pool
        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
        
        void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;
        
        void resetPool();
    
    private:
        LveDevice &lveDevice;
        VkDescriptorPool descriptorPool;
        
        friend class LveDescriptorWriter;
    };
    
    // create descriptor
    class LveDescriptorWriter
    {
    public:
        LveDescriptorWriter(LveDescriptorSetLayout &setLayout, LveDescriptorPool &pool);
        
        LveDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        LveDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);
        
        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);
    
    private:
        LveDescriptorSetLayout &setLayout;
        LveDescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;
    };
 
}  // namespace Vk