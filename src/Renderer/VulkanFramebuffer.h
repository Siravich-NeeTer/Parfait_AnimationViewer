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
				VulkanFramebuffer(const VulkanContext& _vulkanContext, const VulkanSurfaceSwapchain& _vulkanSurfaceSwapchain, const VulkanRenderPass& _vulkanRenderPass, const std::vector<VkImageView>& _attachments = {});
				VulkanFramebuffer(const VulkanContext& _vulkanContext, const VulkanRenderPass& _vulkanRenderPass, uint32_t _width, uint32_t _height, const std::vector<VkImageView>& _attachments = {});
				~VulkanFramebuffer();

				void RecreateFramebuffer(const VulkanSurfaceSwapchain& _vulkanSurfaceSwapchain, const std::vector<VkImageView>& _attachments);
				void RecreateFramebuffer(uint32_t newWidth, uint32_t newHeight, const std::vector<VkImageView>& _attachments);

				const VkFramebuffer& GetFramebuffer() const { return m_Framebuffer; }

			private:
				const VulkanContext& m_VulkanContextRef;
				const VulkanRenderPass& m_VulkanRenderPassRef;

				VkFramebuffer m_Framebuffer;
				std::vector<VkImageView> m_ImageAttachments;
				uint32_t m_Width, m_Height;

				void CreateFramebuffer(uint32_t _width, uint32_t _height);
				void DestroyFramebuffer();
		};
	}
}