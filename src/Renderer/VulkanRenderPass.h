#pragma once

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanSurfaceSwapchain.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanRenderPass
		{
			public:
				VulkanRenderPass(const VulkanContext& _vulkanContext, const VulkanSurfaceSwapchain& _vulkanSurfaceSwapchain);
				~VulkanRenderPass();

				const VkRenderPass& GetRenderPass() const { return m_RenderPass; }

			private:
				const VulkanContext& m_VulkanContextRef;
				const VulkanSurfaceSwapchain& m_VulkanSurfaceSwapchainRef;

				VkRenderPass m_RenderPass;

				void CreateRenderPass();
		};
	}
}