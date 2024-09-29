#pragma once

#include "VulkanContext.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanDescriptor
		{
			public:
				VulkanDescriptor(const VulkanContext& _vulkanContext);
				~VulkanDescriptor();

				void AddLayoutBinding(VkDescriptorSetLayoutBinding layoutBinding);

				void Init();

				void WriteUniformBuffer(uint32_t _binding, VkBuffer _buffer, VkDeviceSize _size, uint32_t _frameIndex);
				void WriteImageBuffer(uint32_t _binding, VkImageView _imageView, VkSampler _sampler, uint32_t _frameIndex);
				void WriteImageArrayBuffer(uint32_t _binding, const std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t _frameIndex);
				void UpdateDescriptorSet();

				const VkDescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
				const VkDescriptorPool& GetDescriptorPool() const { return m_DescriptorPool; }
				const std::vector<VkDescriptorSet>& GetDescriptorSets() const { return m_DescriptorSets; }
				const VkDescriptorSet& GetDescriptorSet(uint32_t _index) const{ return m_DescriptorSets[_index]; }

			private:
				const VulkanContext& m_VulkanContextRef;

				std::vector<VkDescriptorSetLayoutBinding> m_DescriptorSetLayoutBinding;
				VkDescriptorSetLayout m_DescriptorSetLayout;
				VkDescriptorPool m_DescriptorPool;
				std::vector<VkDescriptorSet> m_DescriptorSets;
				std::vector<VkWriteDescriptorSet> m_WriteDescriptorSets;

				std::vector<VkDescriptorBufferInfo> m_BufferInfos;
				std::vector<VkDescriptorImageInfo> m_ImageInfos;

				void CreateDescriptorSetLayout();
				void CreateDescriptorPool();
				void CreateDescriptorSets();
		};
	}
}