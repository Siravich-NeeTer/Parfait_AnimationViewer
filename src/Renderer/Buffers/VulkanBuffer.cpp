#include "VulkanBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		const std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};
		const std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0
		};

		VulkanBuffer::VulkanBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkDeviceSize _size)
			: m_VulkanContextRef(_vulkanContext), m_VulkanCommandPoolRef(_vulkanCommandPool)
		{
			m_BufferSize = _size;

			CreateBuffer(m_BufferSize, _usage, _properties, m_Buffer, m_BufferMemory);
		}
		VulkanBuffer::VulkanBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkDeviceSize _size, const void* _data)
			: m_VulkanContextRef(_vulkanContext), m_VulkanCommandPoolRef(_vulkanCommandPool)
		{
			m_BufferSize = _size;

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			CreateBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory, 0, m_BufferSize, 0, &data);
			memcpy(data, _data, (size_t)m_BufferSize);
			vkUnmapMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory);

			CreateBuffer(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | _usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_BufferMemory);

			CopyBuffer(stagingBuffer, m_Buffer, m_BufferSize);

			vkDestroyBuffer(m_VulkanContextRef.GetLogicalDevice(), stagingBuffer, nullptr);
			vkFreeMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory, nullptr);
		}
		VulkanBuffer::~VulkanBuffer()
		{
			vkDestroyBuffer(m_VulkanContextRef.GetLogicalDevice(), m_Buffer, nullptr);
			vkFreeMemory(m_VulkanContextRef.GetLogicalDevice(), m_BufferMemory, nullptr);
		}

		void VulkanBuffer::CreateBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory)
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = _size;
			bufferInfo.usage = _usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(m_VulkanContextRef.GetLogicalDevice(), &bufferInfo, nullptr, &_buffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to create buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(m_VulkanContextRef.GetLogicalDevice(), _buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, _properties);

			if (vkAllocateMemory(m_VulkanContextRef.GetLogicalDevice(), &allocInfo, nullptr, &_bufferMemory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate buffer memory!");
			}

			vkBindBufferMemory(m_VulkanContextRef.GetLogicalDevice(), _buffer, _bufferMemory, 0);
		}
		void VulkanBuffer::CopyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = m_VulkanCommandPoolRef.GetCommandPool();
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(m_VulkanContextRef.GetLogicalDevice(), &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);
			{
				VkBufferCopy copyRegion{};
				copyRegion.size = _size;
				vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);
			}
			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(m_VulkanContextRef.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(m_VulkanContextRef.GetGraphicsQueue());

			vkFreeCommandBuffers(m_VulkanContextRef.GetLogicalDevice(), m_VulkanCommandPoolRef.GetCommandPool(), 1, &commandBuffer);
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