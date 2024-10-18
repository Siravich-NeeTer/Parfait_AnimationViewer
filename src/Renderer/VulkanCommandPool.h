#pragma once

#include "Renderer/VulkanContext.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanCommandPool
		{
			public:
				VulkanCommandPool(const VulkanContext& _vulkanContextRef);
				~VulkanCommandPool();

				const VkCommandPool GetCommandPool() const { return m_CommandPool; }

			private:
				const VulkanContext& m_VulkanContextRef;

				VkCommandPool m_CommandPool;
				
				void CreateCommandPool();
		};
	}
}