#pragma once

#include "VulkanBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		template<typename Vertex>
		class VulkanVertexBuffer : public VulkanBuffer
		{
			public:
				VulkanVertexBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, const Vertex* _vertices, size_t _vertexCount)
					: VulkanBuffer(_vulkanContext,
						_vulkanCommandPool,
						VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						static_cast<VkDeviceSize>(sizeof(Vertex) * _vertexCount),
						_vertices)
				{

				}
		};
	}
}