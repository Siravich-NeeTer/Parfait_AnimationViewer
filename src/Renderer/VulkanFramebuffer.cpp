#include "VulkanFramebuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanFramebuffer::VulkanFramebuffer(const VulkanContext& _vulkanContext, const VulkanSurfaceSwapchain& _vulkanSurfaceSwapchain, const VulkanRenderPass& _vulkanRenderPass, const std::vector<VkImageView>& _attachments)
			: m_VulkanContextRef(_vulkanContext), 
			m_VulkanSurfaceSwapchainRef(_vulkanSurfaceSwapchain),
			m_VulkanRenderPassRef(_vulkanRenderPass),
			m_ImageAttachments(_attachments)
		{
			CreateFramebuffer();
		}
		VulkanFramebuffer::~VulkanFramebuffer()
		{
			DestroyFramebuffer();
		}

		void VulkanFramebuffer::RecreateFramebuffer(const std::vector<VkImageView>& _attachments)
		{
			DestroyFramebuffer();
			m_ImageAttachments = _attachments;
			CreateFramebuffer();
		}

		void VulkanFramebuffer::CreateFramebuffer()
		{
			const std::vector<VkImageView>& swapchainImageViews = m_VulkanSurfaceSwapchainRef.GetSwapchainImageViews();

			m_Framebuffers.resize(swapchainImageViews.size());
			for (size_t i = 0; i < swapchainImageViews.size(); i++)
			{
				std::vector<VkImageView> attachments = { swapchainImageViews[i] };
				for (size_t j = 0; j < m_ImageAttachments.size(); j++)
				{
					attachments.push_back(m_ImageAttachments[j]);
				}

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = m_VulkanRenderPassRef.GetRenderPass();
				framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				framebufferInfo.pAttachments = attachments.data();
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