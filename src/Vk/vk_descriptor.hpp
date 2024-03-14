// Ref: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/

#pragma once
#include <vulkan/vulkan.h>

// std
#include <vector>
#include <unordered_map>

// ================================================ usage ==========================================
// VkDescriptorSet GlobalSet;
// vkutil::DescriptorBuilder::begin(_descriptorLayoutCache, _descriptorAllocator)
// 	.bind_buffer(0, &dynamicInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT )
// 	.bind_buffer(1, &dynamicInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT)
// 	.build(GlobalSet);

// VkDescriptorSet ObjectDataSet;
// vkutil::DescriptorBuilder::begin(_descriptorLayoutCache, _descriptorAllocator)
// 	.bind_buffer(0, &objectBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
// 	.build(ObjectDataSet);

// VkDescriptorSet ImageSet;
// vkutil::DescriptorBuilder::begin(_descriptorLayoutCache, _descriptorAllocator)
// 		.bind_image(0, &imageBufferInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
// 		.build(ImageSet);


namespace Vk {

    // DescriptorAllocator: manages allocation of descriptor sets. Will keep creating new descriptor pools once they get filled. Can reset the entire thing and reuse pools.
	class DescriptorAllocator {
	public:
        DescriptorAllocator(VkDevice device): device(device) {}
        ~DescriptorAllocator();
		
		struct PoolSizes {
			std::vector<std::pair<VkDescriptorType,float>> sizes =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
			};
		};

        // empty all the pools
		void reset_pools();

        // allocate a descriptor set from pools, depending on input set layout
		bool allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout);

		VkDevice device;
	private:
        
		VkDescriptorPool grab_pool();

		VkDescriptorPool currentPool{VK_NULL_HANDLE};
		PoolSizes descriptorSizes;
		std::vector<VkDescriptorPool> usedPools;
		std::vector<VkDescriptorPool> freePools;
	};

    // DescriptorLayoutCache: caches DescriptorSetLayouts to avoid creating duplicated layouts.
	class DescriptorLayoutCache {
	public:
        DescriptorLayoutCache(VkDevice device): device(device) {}
        ~DescriptorLayoutCache();

		VkDescriptorSetLayout create_descriptor_layout(VkDescriptorSetLayoutCreateInfo* info);

		struct DescriptorLayoutInfo {
			//good idea to turn this into a inlined array
			std::vector<VkDescriptorSetLayoutBinding> bindings;

			bool operator==(const DescriptorLayoutInfo& other) const;

			size_t hash() const;
		};

	private:

		struct DescriptorLayoutHash
		{

			std::size_t operator()(const DescriptorLayoutInfo& k) const
			{
				return k.hash();
			}
		};

		std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> layoutCache;
		VkDevice device;
	};

    // DescriptorBuilder: Uses the 2 objects above to allocate and write a descriptor set and its layout automatically.
	class DescriptorBuilder {
	public:
        DescriptorBuilder(DescriptorLayoutCache& layoutCache, DescriptorAllocator& allocator):
            cache(layoutCache), alloc(allocator) {}
        ~DescriptorBuilder() = default;

		DescriptorBuilder& bind_buffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

		DescriptorBuilder& bind_image(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

        // build set and layout
		bool build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);

        // build set only
		bool build(VkDescriptorSet& set);
	private:
		
		std::vector<VkWriteDescriptorSet> writes;
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		

		DescriptorLayoutCache& cache;
		DescriptorAllocator& alloc;
	};
}

