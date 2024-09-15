#pragma once

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanSurfaceSwapchain.h"
#include "Renderer/VulkanRenderPass.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanFramebuffer
		{
			public:
				VulkanFramebuffer(const VulkanContext& _vulkanContext, const VulkanSurfaceSwapchain& _vulkanSurfaceSwapchain, const VulkanRenderPass& _vulkanRenderPass);
				~VulkanFramebuffer();

				const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_Framebuffers; }

			private:
				const VulkanContext& m_VulkanContextRef;
				const VulkanSurfaceSwapchain& m_VulkanSurfaceSwapchainRef;
				const VulkanRenderPass& m_VulkanRenderPassRef;

				std::vector<VkFramebuffer> m_Framebuffers;

				void CreateFramebuffer();
		};
	}
}