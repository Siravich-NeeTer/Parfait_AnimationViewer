#include "VulkanRenderPass.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanRenderPass::VulkanRenderPass(const VulkanContext& _vulkanContext, const VulkanSurfaceSwapchain& _vulkanSurfaceSwapchain)
			: m_VulkanContextRef(_vulkanContext), m_VulkanSurfaceSwapchainRef(_vulkanSurfaceSwapchain)
		{
			CreateRenderPass();
		}
		VulkanRenderPass::~VulkanRenderPass()
		{
			vkDestroyRenderPass(m_VulkanContextRef.GetLogicalDevice(), m_RenderPass, nullptr);
		}

		void VulkanRenderPass::CreateRenderPass()
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = m_VulkanSurfaceSwapchainRef.GetSurfaceFormat().format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			// TODO: Better Error Handler
			if (vkCreateRenderPass(m_VulkanContextRef.GetLogicalDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to create render pass!");
			}
		}
	}
}