#include "VulkanBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		const std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		};

		const std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};

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

		void VulkanBuffer::CreateBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory)
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = _size;
			bufferInfo.usage = _usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(_vulkanContext.GetLogicalDevice(), &bufferInfo, nullptr, &_buffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to create buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(_vulkanContext.GetLogicalDevice(), _buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(_vulkanContext, memRequirements.memoryTypeBits, _properties);

			if (vkAllocateMemory(_vulkanContext.GetLogicalDevice(), &allocInfo, nullptr, &_bufferMemory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate buffer memory!");
			}

			vkBindBufferMemory(_vulkanContext.GetLogicalDevice(), _buffer, _bufferMemory, 0);
		}
		void VulkanBuffer::CopyBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = _vulkanCommandPool.GetCommandPool();
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(_vulkanContext.GetLogicalDevice(), &allocInfo, &commandBuffer);

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

			vkQueueSubmit(_vulkanContext.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(_vulkanContext.GetGraphicsQueue());

			vkFreeCommandBuffers(_vulkanContext.GetLogicalDevice(), _vulkanCommandPool.GetCommandPool(), 1, &commandBuffer);
		}

		uint32_t VulkanBuffer::FindMemoryType(const VulkanContext& _vulkanContext, uint32_t typeFilter, VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(_vulkanContext.GetPhysicalDevice(), &memProperties);

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