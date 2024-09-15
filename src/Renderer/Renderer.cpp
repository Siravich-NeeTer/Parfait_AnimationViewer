#include "Renderer.h"

namespace Parfait
{
	namespace Graphics
	{
		Renderer::Renderer(GLFWwindow* _window)
			: m_VkContext(new VulkanContext()),
			m_VkSurfaceSwapchain(*m_VkContext, *_window),
			m_VkRenderPass(*m_VkContext, m_VkSurfaceSwapchain),
			m_VkGraphicsPipeline(new VulkanGraphicsPipeline(*m_VkContext, m_VkRenderPass, {"Shaders/temp.vert", "Shaders/temp.frag"})),
			m_VkFramebuffer(new VulkanFramebuffer(*m_VkContext, m_VkSurfaceSwapchain, m_VkRenderPass)),
			m_VkCommandPool(*m_VkContext),
			m_VkCommandBuffer(*m_VkContext, m_VkCommandPool)

		{
			CreateSyncObject();
		}
		Renderer::~Renderer()
		{
			DestroySyncObject();
		}

		void Renderer::Draw()
		{
			vkWaitForFences(m_VkContext->GetLogicalDevice(), 1, &m_Fence, VK_TRUE, UINT64_MAX);
			vkResetFences(m_VkContext->GetLogicalDevice(), 1, &m_Fence);

			uint32_t imageIndex;
			vkAcquireNextImageKHR(m_VkContext->GetLogicalDevice(), m_VkSurfaceSwapchain.GetSwapchain(), UINT64_MAX, m_PresentSemaphore, VK_NULL_HANDLE, &imageIndex);

			vkResetCommandBuffer(m_VkCommandBuffer.GetCommandBuffer(), /*VkCommandBufferResetFlagBits*/ 0);
			BeginRenderPass(m_VkCommandBuffer, imageIndex);
			{
				vkCmdBindPipeline(m_VkCommandBuffer.GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkGraphicsPipeline->GetPipeline());

				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)m_VkSurfaceSwapchain.GetExtent().width;
				viewport.height = (float)m_VkSurfaceSwapchain.GetExtent().height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(m_VkCommandBuffer.GetCommandBuffer(), 0, 1, &viewport);

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = m_VkSurfaceSwapchain.GetExtent();
				vkCmdSetScissor(m_VkCommandBuffer.GetCommandBuffer(), 0, 1, &scissor);

				vkCmdDraw(m_VkCommandBuffer.GetCommandBuffer(), 3, 1, 0, 0);
			}
			EndRenderPass(m_VkCommandBuffer);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { m_PresentSemaphore };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_VkCommandBuffer.GetCommandBuffer();

			VkSemaphore signalSemaphores[] = { m_RenderSemaphore };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			// TODO: Better Error Handler
			if (vkQueueSubmit(m_VkContext->GetGraphicsQueue(), 1, &submitInfo, m_Fence) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to submit draw command buffer!");
			}

			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { m_VkSurfaceSwapchain.GetSwapchain()};
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &imageIndex;

			vkQueuePresentKHR(m_VkContext->GetPresentQueue(), &presentInfo);
		}

		void Renderer::BeginRenderPass(const VulkanCommandBuffer& _VkCommandBuffer, uint32_t _imageIndex)
		{
			_VkCommandBuffer.Begin();

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_VkRenderPass.GetRenderPass();
			renderPassInfo.framebuffer = m_VkFramebuffer->GetFramebuffers()[_imageIndex];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_VkSurfaceSwapchain.GetExtent();

			VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(_VkCommandBuffer.GetCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		}
		void Renderer::EndRenderPass(const VulkanCommandBuffer& _VkCommandBuffer)
		{
			vkCmdEndRenderPass(_VkCommandBuffer.GetCommandBuffer());
			_VkCommandBuffer.End();
		}

		void Renderer::CreateSyncObject()
		{
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(m_VkContext->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_PresentSemaphore) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create semaphore");
			}
			if (vkCreateSemaphore(m_VkContext->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderSemaphore) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create semaphore");
			}

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			if (vkCreateFence(m_VkContext->GetLogicalDevice(), &fenceInfo, nullptr, &m_Fence) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create fence");
			}

		}
		void Renderer::DestroySyncObject()
		{
			vkDestroySemaphore(m_VkContext->GetLogicalDevice(), m_PresentSemaphore, nullptr);
			vkDestroySemaphore(m_VkContext->GetLogicalDevice(), m_RenderSemaphore, nullptr);

			vkDestroyFence(m_VkContext->GetLogicalDevice(), m_Fence, nullptr);
		}
	}
}