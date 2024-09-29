#include "VulkanBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanBuffer::VulkanBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkDeviceSize _size)
			: m_VulkanContextRef(_vulkanContext), m_VulkanCommandPoolRef(_vulkanCommandPool)
		{
			m_BufferSize = _size;

			CreateBuffer(_vulkanContext, _vulkanCommandPool, m_BufferSize, _usage, _properties, m_Buffer, m_BufferMemory);
		}
		VulkanBuffer::VulkanBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkDeviceSize _size, const void* _data)
			: m_VulkanContextRef(_vulkanContext), m_VulkanCommandPoolRef(_vulkanCommandPool)
		{
			m_BufferSize = _size;

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			CreateBuffer(_vulkanContext, _vulkanCommandPool, m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory, 0, m_BufferSize, 0, &data);
			memcpy(data, _data, (size_t)m_BufferSize);
			vkUnmapMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory);

			CreateBuffer(_vulkanContext, _vulkanCommandPool, m_BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | _usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_BufferMemory);

			CopyBuffer(_vulkanContext, _vulkanCommandPool, stagingBuffer, m_Buffer, m_BufferSize);

			vkDestroyBuffer(m_VulkanContextRef.GetLogicalDevice(), stagingBuffer, nullptr);
			vkFreeMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory, nullptr);
		}
		VulkanBuffer::~VulkanBuffer()
		{
			vkDestroyBuffer(m_VulkanContextRef.GetLogicalDevice(), m_Buffer, nullptr);
			vkFreeMemory(m_VulkanContextRef.GetLogicalDevice(), m_BufferMemory, nullptr);
		}
	}
}