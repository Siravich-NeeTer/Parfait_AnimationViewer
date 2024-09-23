#include "VulkanDescriptor.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanDescriptor::VulkanDescriptor(const VulkanContext& _vulkanContext)
			: m_VulkanContextRef(_vulkanContext)
		{
			/*
			CreateDescriptorSetLayout();
			CreateDescriptorPool();
			CreateDescriptorSets();
			*/
		}
		VulkanDescriptor::~VulkanDescriptor()
		{
			vkDestroyDescriptorPool(m_VulkanContextRef.GetLogicalDevice(), m_DescriptorPool, nullptr);
			vkDestroyDescriptorSetLayout(m_VulkanContextRef.GetLogicalDevice(), m_DescriptorSetLayout, nullptr);
		}

		void VulkanDescriptor::AddLayoutBinding(VkDescriptorSetLayoutBinding layoutBinding)
		{
			m_DescriptorSetLayoutBinding.push_back(layoutBinding);
		}

		void VulkanDescriptor::Init()
		{
			CreateDescriptorSetLayout();
			CreateDescriptorPool();
			CreateDescriptorSets();
		}

		void VulkanDescriptor::WriteUniformBuffer(uint32_t _binding, VkBuffer _buffer, VkDeviceSize _size, size_t _frameIndex)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = _buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = _size;
			m_BufferInfos.push_back(bufferInfo);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[_frameIndex];
			descriptorWrite.dstBinding = _binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &m_BufferInfos.back();

			m_WriteDescriptorSets.push_back(descriptorWrite);
		}
		void VulkanDescriptor::WriteImageBuffer(uint32_t _binding, VkImageView _imageView, VkSampler _sampler, uint32_t _frameIndex)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = _imageView;
			imageInfo.sampler = _sampler;
			m_ImageInfos.push_back(imageInfo);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[_frameIndex];
			descriptorWrite.dstBinding = 1;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &m_ImageInfos.back();

			m_WriteDescriptorSets.push_back(descriptorWrite);
		}
		void VulkanDescriptor::UpdateDescriptorSet()
		{
			vkUpdateDescriptorSets(m_VulkanContextRef.GetLogicalDevice(), static_cast<uint32_t>(m_WriteDescriptorSets.size()), m_WriteDescriptorSets.data(), 0, nullptr);
			m_WriteDescriptorSets.clear();
			m_BufferInfos.clear();
			m_ImageInfos.clear();
		}

		void VulkanDescriptor::CreateDescriptorSetLayout()
		{
			/*
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = 0;
			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			*/

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(m_DescriptorSetLayoutBinding.size());
			layoutInfo.pBindings = m_DescriptorSetLayoutBinding.data();

			if (vkCreateDescriptorSetLayout(m_VulkanContextRef.GetLogicalDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to create descriptor set layout!");
			}
		}
		void VulkanDescriptor::CreateDescriptorPool()
		{
			std::vector<VkDescriptorPoolSize> poolSizes(m_DescriptorSetLayoutBinding.size());
			for (size_t i = 0; i < m_DescriptorSetLayoutBinding.size(); i++)
			{
				poolSizes[i].type = m_DescriptorSetLayoutBinding[i].descriptorType;
				poolSizes[i].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
			}

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

			if (vkCreateDescriptorPool(m_VulkanContextRef.GetLogicalDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to create descriptor pool!");
			}
		}
		void VulkanDescriptor::CreateDescriptorSets()
		{
			std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_DescriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
			allocInfo.pSetLayouts = layouts.data();

			m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
			if (vkAllocateDescriptorSets(m_VulkanContextRef.GetLogicalDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to allocate descriptor sets!");
			}
		}
	}
}