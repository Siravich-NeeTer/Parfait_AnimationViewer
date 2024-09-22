#pragma once

#include "VulkanBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		template <typename Material>
		class VulkanUniformBuffer : public VulkanBuffer
		{
			public:
				VulkanUniformBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool)
					: VulkanBuffer(_vulkanContext,
						_vulkanCommandPool,
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						static_cast<VkDeviceSize>(sizeof(Material)))
				{
					
				}

				void*& GetMappedBuffer() { return m_MappedBuffer; }

			private:
				void* m_MappedBuffer;
		};
	}
}