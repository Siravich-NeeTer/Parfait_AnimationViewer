#include "VulkanCommandBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanCommandBuffer::VulkanCommandBuffer(const VulkanContext& _vulkanContextRef, const VulkanCommandPool& _vulkanCommandPool)
			: m_VulkanContextRef(_vulkanContextRef),
			m_VulkanCommandPool(_vulkanCommandPool)
		{
			CreateCommandBuffer();
		}
		VulkanCommandBuffer::~VulkanCommandBuffer()
		{

		}

		void VulkanCommandBuffer::Begin() const
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			// TODO: Better Error Handler
			if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to begin recording command buffer!");
			}
		}
		void VulkanCommandBuffer::End() const
		{
			// TODO: Better Error Handler
			if (vkEndCommandBuffer(m_CommandBuffer) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to end record command buffer!");
			}
		}

		void VulkanCommandBuffer::CreateCommandBuffer()
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_VulkanCommandPool.GetCommandPool();
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			// TODO: Better Error Handler
			if (vkAllocateCommandBuffers(m_VulkanContextRef.GetLogicalDevice(), &allocInfo, &m_CommandBuffer) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to allocate command buffers!");
			}
		}
	}
}