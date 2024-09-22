#pragma once

#include "VulkanContext.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanDescriptor
		{
			public:
				VulkanDescriptor(const VulkanContext& _vulkanContext, uint32_t _descriptorSize);
				~VulkanDescriptor();

				void WriteBuffer(uint32_t _binding, VkBuffer _buffer, VkDeviceSize _size, uint32_t _frameIndex);

				const VkDescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
				const VkDescriptorPool& GetDescriptorPool() const { return m_DescriptorPool; }
				const std::vector<VkDescriptorSet>& GetDescriptorSets() const { return m_DescriptorSets; }
				const VkDescriptorSet& GetDescriptorSet(uint32_t _index) const{ return m_DescriptorSets[_index]; }

			private:
				const VulkanContext& m_VulkanContextRef;

				VkDescriptorSetLayout m_DescriptorSetLayout;
				VkDescriptorPool m_DescriptorPool;
				std::vector<VkDescriptorSet> m_DescriptorSets;

				uint32_t m_DescriptorSize;

				void CreateDescriptorSetLayout();
				void CreateDescriptorPool();
				void CreateDescriptorSets();
		};
	}
}