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
				void AddDescriptorSets(std::vector<VkDescriptorSetLayoutBinding> layoutBindings);

				void SetCurrentDescriptorIndex(uint32_t _index) { m_CurrentDescriptorSetIndex = _index; }

				void Init();

				void WriteUniformBuffer(uint32_t _binding, VkBuffer _buffer, VkDeviceSize _size, uint32_t _frameIndex);
				void WriteImageBuffer(uint32_t _binding, VkImageView _imageView, VkSampler _sampler, uint32_t _frameIndex);
				void WriteImageArrayBuffer(uint32_t _binding, const std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t _frameIndex);
				void UpdateDescriptorSet();

				const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
				const VkDescriptorSetLayout& GetDescriptorSetLayout(uint32_t _index) const { return m_DescriptorSetLayout[_index]; }
				const VkDescriptorPool& GetDescriptorPool(uint32_t _index) const { return m_DescriptorPool[_index]; }
				const std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>& GetDescriptorSets(uint32_t _index) const { return m_DescriptorSets[_index]; }
				// const VkDescriptorSet& GetDescriptorSet(uint32_t _index) const{ return m_DescriptorSets[_index]; }

			private:
				const VulkanContext& m_VulkanContextRef;

				std::vector<std::vector<VkDescriptorSetLayoutBinding>> m_DescriptorSetLayoutBinding;
				std::vector<VkDescriptorSetLayout> m_DescriptorSetLayout;
				std::vector<VkDescriptorPool> m_DescriptorPool;
				std::vector<std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>> m_DescriptorSets;
				std::vector<VkWriteDescriptorSet> m_WriteDescriptorSets;

				std::vector<VkDescriptorBufferInfo> m_BufferInfos;
				std::vector<VkDescriptorImageInfo> m_ImageInfos;
				uint32_t m_CurrentDescriptorSetIndex;

				VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& _descriptorSetLayoutBinding);
				VkDescriptorPool CreateDescriptorPool(const std::vector<VkDescriptorSetLayoutBinding>& _descriptorSetLayoutBinding);
				std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> CreateDescriptorSets(VkDescriptorSetLayout _descriptorSetLayout, VkDescriptorPool _descriptorPool);
		};
	}
}