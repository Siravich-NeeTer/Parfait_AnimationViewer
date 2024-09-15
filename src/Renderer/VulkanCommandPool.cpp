#include "VulkanCommandPool.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanCommandPool::VulkanCommandPool(const VulkanContext& _vulkanContextRef)
			: m_VulkanContextRef(_vulkanContextRef)
		{
			CreateCommandPool();
		}
		VulkanCommandPool::~VulkanCommandPool()
		{
			vkDestroyCommandPool(m_VulkanContextRef.GetLogicalDevice(), m_CommandPool, nullptr);
		}

		void VulkanCommandPool::CreateCommandPool()
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = m_VulkanContextRef.GetGraphicsQueueFamily();

			// TODO: Better Error Handler
			if (vkCreateCommandPool(m_VulkanContextRef.GetLogicalDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create command pool!");
			}
		}
	}
}