#include "Renderer.h"

namespace Parfait
{
	namespace Graphics
	{
		Renderer::Renderer(GLFWwindow* _window)
			: m_VkContext(new VulkanContext()),
			m_VkSurfaceSwapchain(new VulkanSurfaceSwapchain(*m_VkContext, *_window)),
			m_VkRenderPass(new VulkanRenderPass(*m_VkContext, *m_VkSurfaceSwapchain)),
			m_VkGraphicsPipeline(new VulkanGraphicsPipeline(*m_VkContext, *m_VkRenderPass, {"Shaders/temp.vert", "Shaders/temp.frag"})),
			m_VkFramebuffer(new VulkanFramebuffer(*m_VkContext, *m_VkSurfaceSwapchain, *m_VkRenderPass)),
			m_VkCommandPool(*m_VkContext)

		{
			CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT);
			CreateSyncObject(MAX_FRAMES_IN_FLIGHT);
		}
		Renderer::~Renderer()
		{
			vkDeviceWaitIdle(m_VkContext->GetLogicalDevice());
			DestroySyncObject();
		}

		void Renderer::Draw()
		{
			vkWaitForFences(m_VkContext->GetLogicalDevice(), 1, &m_InflightFence[m_CurrentFrame], VK_TRUE, UINT64_MAX);
			vkResetFences(m_VkContext->GetLogicalDevice(), 1, &m_InflightFence[m_CurrentFrame]);

			uint32_t imageIndex;
			vkAcquireNextImageKHR(m_VkContext->GetLogicalDevice(), m_VkSurfaceSwapchain->GetSwapchain(), UINT64_MAX, m_PresentSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

			vkResetCommandBuffer(m_VkCommandBuffers[m_CurrentFrame]->GetCommandBuffer(), /*VkCommandBufferResetFlagBits*/ 0);
			BeginRenderPass(*m_VkCommandBuffers[m_CurrentFrame], imageIndex);
			{
				vkCmdBindPipeline(m_VkCommandBuffers[m_CurrentFrame]->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkGraphicsPipeline->GetPipeline());

				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)m_VkSurfaceSwapchain->GetExtent().width;
				viewport.height = (float)m_VkSurfaceSwapchain->GetExtent().height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(m_VkCommandBuffers[m_CurrentFrame]->GetCommandBuffer(), 0, 1, &viewport);

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = m_VkSurfaceSwapchain->GetExtent();
				vkCmdSetScissor(m_VkCommandBuffers[m_CurrentFrame]->GetCommandBuffer(), 0, 1, &scissor);

				vkCmdDraw(m_VkCommandBuffers[m_CurrentFrame]->GetCommandBuffer(), 3, 1, 0, 0);
			}
			EndRenderPass(*m_VkCommandBuffers[m_CurrentFrame]);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { m_PresentSemaphores[m_CurrentFrame]};
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_VkCommandBuffers[m_CurrentFrame]->GetCommandBuffer();

			VkSemaphore signalSemaphores[] = { m_RenderSemaphores[m_CurrentFrame]};
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			// TODO: Better Error Handler
			if (vkQueueSubmit(m_VkContext->GetGraphicsQueue(), 1, &submitInfo, m_InflightFence[m_CurrentFrame]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to submit draw command buffer!");
			}

			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapChains[] = { m_VkSurfaceSwapchain->GetSwapchain()};
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &imageIndex;

			vkQueuePresentKHR(m_VkContext->GetPresentQueue(), &presentInfo);

			m_CurrentFrame = (m_CurrentFrame + 1) / MAX_FRAMES_IN_FLIGHT;
		}

		void Renderer::BeginRenderPass(const VulkanCommandBuffer& _VkCommandBuffer, uint32_t _imageIndex)
		{
			_VkCommandBuffer.Begin();

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_VkRenderPass->GetRenderPass();
			renderPassInfo.framebuffer = m_VkFramebuffer->GetFramebuffers()[_imageIndex];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_VkSurfaceSwapchain->GetExtent();

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

		void Renderer::CreateCommandBuffers(uint32_t _size)
		{
			m_VkCommandBuffers.resize(_size);
			for (uint32_t i = 0; i < _size; i++)
			{
				m_VkCommandBuffers[i] = std::make_unique<VulkanCommandBuffer>(*m_VkContext, m_VkCommandPool);
			}
		}
		void Renderer::RecreateSwapchain()
		{

		}

		void Renderer::CreateSyncObject(uint32_t _size)
		{
			m_PresentSemaphores.resize(_size);
			m_RenderSemaphores.resize(_size);
			m_InflightFence.resize(_size);

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			
			for (size_t i = 0; i < _size; i++)
			{
				if (vkCreateSemaphore(m_VkContext->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_PresentSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(m_VkContext->GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderSemaphores[i]) != VK_SUCCESS)
				{
					throw std::runtime_error("Failed to create semaphore");
				}

				if (vkCreateFence(m_VkContext->GetLogicalDevice(), &fenceInfo, nullptr, &m_InflightFence[i]) != VK_SUCCESS)
				{
					throw std::runtime_error("Failed to create fence");
				}
			}


		}
		void Renderer::DestroySyncObject()
		{
			for (size_t i = 0; i < m_PresentSemaphores.size(); i++)
			{
				vkDestroySemaphore(m_VkContext->GetLogicalDevice(), m_PresentSemaphores[i], nullptr);
			}
			for (size_t i = 0; i < m_RenderSemaphores.size(); i++)
			{
				vkDestroySemaphore(m_VkContext->GetLogicalDevice(), m_RenderSemaphores[i], nullptr);
			}
			for (size_t i = 0; i < m_InflightFence.size(); i++)
			{
				vkDestroyFence(m_VkContext->GetLogicalDevice(), m_InflightFence[i], nullptr);
			}

		}
	}
}