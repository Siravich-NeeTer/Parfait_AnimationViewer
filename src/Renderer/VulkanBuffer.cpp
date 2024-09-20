#include "VulkanBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanBuffer::VulkanBuffer(const VulkanContext& _vulkanContext)
			: m_VulkanContextRef(_vulkanContext)
		{
			CreateBuffer();
		}
		VulkanBuffer::~VulkanBuffer()
		{
			vkDestroyBuffer(m_VulkanContextRef.GetLogicalDevice(), m_Buffer, nullptr);
			vkFreeMemory(m_VulkanContextRef.GetLogicalDevice(), m_BufferMemory, nullptr);
		}

		void VulkanBuffer::CreateBuffer()
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = sizeof(vertices[0]) * vertices.size();
			// TODO: Change this usage later to support Index Buffer?
			bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(m_VulkanContextRef.GetLogicalDevice(), &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create vertex buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(m_VulkanContextRef.GetLogicalDevice(), m_Buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			if (vkAllocateMemory(m_VulkanContextRef.GetLogicalDevice(), &allocInfo, nullptr, &m_BufferMemory) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to allocate vertex buffer memory!");
			}
			vkBindBufferMemory(m_VulkanContextRef.GetLogicalDevice(), m_Buffer, m_BufferMemory, 0);

			void* data;
			vkMapMemory(m_VulkanContextRef.GetLogicalDevice(), m_BufferMemory, 0, bufferInfo.size, 0, &data);
			memcpy(data, vertices.data(), (size_t)bufferInfo.size);
			vkUnmapMemory(m_VulkanContextRef.GetLogicalDevice(), m_BufferMemory);
		}

		uint32_t VulkanBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(m_VulkanContextRef.GetPhysicalDevice(), &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
			{
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
				{
					return i;
				}
			}

			throw std::runtime_error("Failed to find suitable memory type!");
		}
	}
}