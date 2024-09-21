#pragma once

#include "VulkanBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanIndexBuffer : public VulkanBuffer
		{
			public:
				VulkanIndexBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, const uint16_t* _indices, size_t _indexCount)
					: VulkanBuffer(_vulkanContext,
						_vulkanCommandPool,
						VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						static_cast<VkDeviceSize>(sizeof(uint16_t) * _indexCount),
						_indices)
				{

				}
		};
	}
}