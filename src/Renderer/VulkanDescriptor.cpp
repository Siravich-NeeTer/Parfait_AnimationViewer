#include "VulkanDescriptor.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanDescriptor::VulkanDescriptor(const VulkanContext& _vulkanContext, uint32_t _descriptorSize)
			: m_VulkanContextRef(_vulkanContext),
			m_DescriptorSize(_descriptorSize)
		{
			CreateDescriptorSetLayout();
			CreateDescriptorPool();
			CreateDescriptorSets();
		}
		VulkanDescriptor::~VulkanDescriptor()
		{
			vkDestroyDescriptorPool(m_VulkanContextRef.GetLogicalDevice(), m_DescriptorPool, nullptr);
			vkDestroyDescriptorSetLayout(m_VulkanContextRef.GetLogicalDevice(), m_DescriptorSetLayout, nullptr);
		}

		void VulkanDescriptor::WriteBuffer(uint32_t _binding, VkBuffer _buffer, VkDeviceSize _size, size_t _frameIndex)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = _buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = _size;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[_frameIndex];
			descriptorWrite.dstBinding = _binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(m_VulkanContextRef.GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
		}

		void VulkanDescriptor::CreateDescriptorSetLayout()
		{
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = 0;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &layoutBinding;

			if (vkCreateDescriptorSetLayout(m_VulkanContextRef.GetLogicalDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to create descriptor set layout!");
			}
		}
		void VulkanDescriptor::CreateDescriptorPool()
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = m_DescriptorSize;

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = 1;
			poolInfo.pPoolSizes = &poolSize;
			poolInfo.maxSets = m_DescriptorSize;

			if (vkCreateDescriptorPool(m_VulkanContextRef.GetLogicalDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to create descriptor pool!");
			}
		}
		void VulkanDescriptor::CreateDescriptorSets()
		{
			std::vector<VkDescriptorSetLayout> layouts(m_DescriptorSize, m_DescriptorSetLayout);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_DescriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(m_DescriptorSize);
			allocInfo.pSetLayouts = layouts.data();

			m_DescriptorSets.resize(m_DescriptorSize);
			if (vkAllocateDescriptorSets(m_VulkanContextRef.GetLogicalDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to allocate descriptor sets!");
			}
		}
	}
}