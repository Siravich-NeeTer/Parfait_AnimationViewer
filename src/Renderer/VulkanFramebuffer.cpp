#include "VulkanFramebuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanFramebuffer::VulkanFramebuffer(const VulkanContext& _vulkanContext, const VulkanSurfaceSwapchain& _vulkanSurfaceSwapchain, const VulkanRenderPass& _vulkanRenderPass)
			: m_VulkanContextRef(_vulkanContext), 
			m_VulkanSurfaceSwapchainRef(_vulkanSurfaceSwapchain),
			m_VulkanRenderPassRef(_vulkanRenderPass)
		{
			CreateFramebuffer();
		}
		VulkanFramebuffer::~VulkanFramebuffer()
		{
			DestroyFramebuffer();
		}

		void VulkanFramebuffer::RecreateFramebuffer()
		{
			DestroyFramebuffer();
			CreateFramebuffer();
		}

		void VulkanFramebuffer::CreateFramebuffer()
		{
			const std::vector<VkImageView>& swapchainImageViews = m_VulkanSurfaceSwapchainRef.GetSwapchainImageViews();

			m_Framebuffers.resize(swapchainImageViews.size());
			for (size_t i = 0; i < swapchainImageViews.size(); i++)
			{
				VkImageView attachments[] = 
				{
					swapchainImageViews[i]
				};

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = m_VulkanRenderPassRef.GetRenderPass();
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = m_VulkanSurfaceSwapchainRef.GetExtent().width;
				framebufferInfo.height = m_VulkanSurfaceSwapchainRef.GetExtent().height;
				framebufferInfo.layers = 1;

				// TODO: Better Error Handler
				if (vkCreateFramebuffer(m_VulkanContextRef.GetLogicalDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS) 
				{
					throw std::runtime_error("Failed to create framebuffer!");
				}
			}
		}
		void VulkanFramebuffer::DestroyFramebuffer()
		{
			for (const VkFramebuffer& framebuffer : m_Framebuffers)
			{
				vkDestroyFramebuffer(m_VulkanContextRef.GetLogicalDevice(), framebuffer, nullptr);
			}
		}
	}
}