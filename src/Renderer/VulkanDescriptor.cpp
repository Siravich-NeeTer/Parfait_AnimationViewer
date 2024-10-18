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
			for(size_t i = 0; i < m_DescriptorPool.size(); i++)
				vkDestroyDescriptorPool(m_VulkanContextRef.GetLogicalDevice(), m_DescriptorPool[i], nullptr);
			for (size_t i = 0; i < m_DescriptorSetLayout.size(); i++)
				vkDestroyDescriptorSetLayout(m_VulkanContextRef.GetLogicalDevice(), m_DescriptorSetLayout[i], nullptr);
		}

		void VulkanDescriptor::AddLayoutBinding(VkDescriptorSetLayoutBinding layoutBinding)
		{
			if (m_DescriptorSetLayoutBinding.empty())
			{
				m_DescriptorSetLayoutBinding.push_back({ layoutBinding });
				m_CurrentDescriptorSetIndex = 0;
			}
			else
			{
				m_DescriptorSetLayoutBinding[m_CurrentDescriptorSetIndex].push_back(layoutBinding);
			}
		}
		void VulkanDescriptor::AddDescriptorSets(std::vector<VkDescriptorSetLayoutBinding> layoutBindings)
		{
			m_DescriptorSetLayoutBinding.push_back(layoutBindings);
			m_CurrentDescriptorSetIndex = m_DescriptorSetLayoutBinding.size() - 1;
		}

		void VulkanDescriptor::Init()
		{
			for (size_t i = 0; i < m_DescriptorSetLayoutBinding.size(); i++)
			{
				VkDescriptorSetLayout descriptorSetLayout = CreateDescriptorSetLayout(m_DescriptorSetLayoutBinding[i]);
				VkDescriptorPool descriptorPool = CreateDescriptorPool(m_DescriptorSetLayoutBinding[i]);
				std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSet = CreateDescriptorSets(descriptorSetLayout, descriptorPool);

				m_DescriptorSetLayout.push_back(descriptorSetLayout);
				m_DescriptorPool.push_back(descriptorPool);
				m_DescriptorSets.push_back(descriptorSet);
			}
		}

		void VulkanDescriptor::WriteUniformBuffer(uint32_t _binding, VkBuffer _buffer, VkDeviceSize _size, uint32_t _frameIndex)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = _buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = _size;
			m_BufferInfos.push_back(bufferInfo);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[m_CurrentDescriptorSetIndex][_frameIndex];
			descriptorWrite.dstBinding = _binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &m_BufferInfos.back();

			m_WriteDescriptorSets.push_back(descriptorWrite);
		}
		void VulkanDescriptor::WriteStorageBuffer(uint32_t _binding, VkBuffer _buffer, VkDeviceSize _size, uint32_t _frameIndex)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = _buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = _size;
			m_BufferInfos.push_back(bufferInfo);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[m_CurrentDescriptorSetIndex][_frameIndex];
			descriptorWrite.dstBinding = _binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
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
			descriptorWrite.dstSet = m_DescriptorSets[m_CurrentDescriptorSetIndex][_frameIndex];
			descriptorWrite.dstBinding = _binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &m_ImageInfos.back();

			m_WriteDescriptorSets.push_back(descriptorWrite);
		}
		void VulkanDescriptor::WriteImageArrayBuffer(uint32_t _binding, const std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t _frameIndex)
		{
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[m_CurrentDescriptorSetIndex][_frameIndex];
			descriptorWrite.dstBinding = _binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());
			descriptorWrite.pImageInfo = imageInfos.data();

			m_WriteDescriptorSets.push_back(descriptorWrite);
		}
		void VulkanDescriptor::UpdateDescriptorSet()
		{
			vkUpdateDescriptorSets(m_VulkanContextRef.GetLogicalDevice(), static_cast<uint32_t>(m_WriteDescriptorSets.size()), m_WriteDescriptorSets.data(), 0, nullptr);
			m_WriteDescriptorSets.clear();
			m_BufferInfos.clear();
			m_ImageInfos.clear();
		}

		VkDescriptorSetLayout VulkanDescriptor::CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& _descriptorSetLayoutBinding)
		{
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(_descriptorSetLayoutBinding.size());
			layoutInfo.pBindings = _descriptorSetLayoutBinding.data();

			VkDescriptorSetLayout descriptorSetLayout;
			if (vkCreateDescriptorSetLayout(m_VulkanContextRef.GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create descriptor set layout!");
			}
			return descriptorSetLayout;
		}
		VkDescriptorPool VulkanDescriptor::CreateDescriptorPool(const std::vector<VkDescriptorSetLayoutBinding>& _descriptorSetLayoutBinding)
		{
			std::vector<VkDescriptorPoolSize> poolSizes(_descriptorSetLayoutBinding.size());
			for (size_t i = 0; i < _descriptorSetLayoutBinding.size(); i++)
			{
				poolSizes[i].type = _descriptorSetLayoutBinding[i].descriptorType;
				poolSizes[i].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
			}

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

			VkDescriptorPool descriptorPool;
			if (vkCreateDescriptorPool(m_VulkanContextRef.GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create descriptor pool!");
			}
			return descriptorPool;
		}
		std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> VulkanDescriptor::CreateDescriptorSets(VkDescriptorSetLayout _descriptorSetLayout, VkDescriptorPool _descriptorPool)
		{
			std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = _descriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
			allocInfo.pSetLayouts = layouts.data();

			std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets;
			if (vkAllocateDescriptorSets(m_VulkanContextRef.GetLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to allocate descriptor sets!");
			}
			return descriptorSets;
		}
	}
}