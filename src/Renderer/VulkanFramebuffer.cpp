#include "VulkanFramebuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanFramebuffer::VulkanFramebuffer(const VulkanContext& _vulkanContext, const VulkanSurfaceSwapchain& _vulkanSurfaceSwapchain, const VulkanRenderPass& _vulkanRenderPass, const std::vector<VkImageView>& _attachments)
			: m_VulkanContextRef(_vulkanContext),
			m_VulkanRenderPassRef(_vulkanRenderPass),
			m_ImageAttachments(_attachments)
		{
			CreateFramebuffer(_vulkanSurfaceSwapchain.GetExtent().width, _vulkanSurfaceSwapchain.GetExtent().height);
		}
		VulkanFramebuffer::VulkanFramebuffer(const VulkanContext& _vulkanContext, const VulkanRenderPass& _vulkanRenderPass, uint32_t _width, uint32_t _height, const std::vector<VkImageView>& _attachments)
			: m_VulkanContextRef(_vulkanContext),
			m_VulkanRenderPassRef(_vulkanRenderPass),
			m_ImageAttachments(_attachments)
		{
			CreateFramebuffer(_width, _height);
		}
		VulkanFramebuffer::~VulkanFramebuffer()
		{
			DestroyFramebuffer();
		}

		void VulkanFramebuffer::RecreateFramebuffer(const VulkanSurfaceSwapchain& _vulkanSurfaceSwapchain, const std::vector<VkImageView>& _attachments)
		{
			RecreateFramebuffer(_vulkanSurfaceSwapchain.GetExtent().width, _vulkanSurfaceSwapchain.GetExtent().height, _attachments);
		}
		void VulkanFramebuffer::RecreateFramebuffer(uint32_t newWidth, uint32_t newHeight, const std::vector<VkImageView>& _attachments)
		{
			DestroyFramebuffer();
			m_ImageAttachments = _attachments;
			CreateFramebuffer(newWidth, newHeight);
		}

		void VulkanFramebuffer::CreateFramebuffer(uint32_t _width, uint32_t _height)
		{
			m_Width = _width;
			m_Height = _height;

			std::vector<VkImageView> attachments;
			for (size_t j = 0; j < m_ImageAttachments.size(); j++)
			{
				attachments.push_back(m_ImageAttachments[j]);
			}

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_VulkanRenderPassRef.GetRenderPass();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = _width;
			framebufferInfo.height = _height;
			framebufferInfo.layers = 1;

			// TODO: Better Error Handler
			if (vkCreateFramebuffer(m_VulkanContextRef.GetLogicalDevice(), &framebufferInfo, nullptr, &m_Framebuffer) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}
		void VulkanFramebuffer::DestroyFramebuffer()
		{
			vkDestroyFramebuffer(m_VulkanContextRef.GetLogicalDevice(), m_Framebuffer, nullptr);
		}
	}
}