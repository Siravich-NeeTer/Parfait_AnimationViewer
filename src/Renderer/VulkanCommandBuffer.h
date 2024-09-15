#pragma once

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanCommandPool.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanCommandBuffer
		{
			public:
				VulkanCommandBuffer(const VulkanContext& _vulkanContextRef, const VulkanCommandPool& _vulkanCommandPool);
				~VulkanCommandBuffer();

				void Begin() const;
				void End() const;

				const VkCommandBuffer& GetCommandBuffer() const { return m_CommandBuffer; }

			private:
				const VulkanContext& m_VulkanContextRef;
				const VulkanCommandPool& m_VulkanCommandPool;

				VkCommandBuffer m_CommandBuffer;

				void CreateCommandBuffer();
		};
	}
}