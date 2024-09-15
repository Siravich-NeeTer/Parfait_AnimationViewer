#pragma once

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanSurfaceSwapchain.h"
#include "Renderer/VulkanRenderPass.h"
#include "Renderer/VulkanGraphicsPipeline.h"
#include "Renderer/VulkanFramebuffer.h"
#include "Renderer/VulkanCommandPool.h"
#include "Renderer/VulkanCommandBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		const int MAX_FRAMES_IN_FLIGHT = 2;

		class Renderer
		{
			public:
				Renderer(GLFWwindow* _window);
				~Renderer();

				void Draw();

				void BeginRenderPass(const VulkanCommandBuffer& _VkCommandBuffer, uint32_t _imageIndex);
				void EndRenderPass(const VulkanCommandBuffer& _VkCommandBuffer);

			private:
				std::unique_ptr<VulkanContext> m_VkContext;
				std::unique_ptr<VulkanSurfaceSwapchain> m_VkSurfaceSwapchain;
				std::unique_ptr<VulkanRenderPass> m_VkRenderPass;
				std::unique_ptr<VulkanGraphicsPipeline> m_VkGraphicsPipeline;
				std::unique_ptr<VulkanFramebuffer> m_VkFramebuffer;
				VulkanCommandPool m_VkCommandPool;
				std::vector<std::unique_ptr<VulkanCommandBuffer>> m_VkCommandBuffers;

				std::vector<VkSemaphore> m_PresentSemaphores;
				std::vector<VkSemaphore> m_RenderSemaphores;
				std::vector<VkFence> m_InflightFence;

				int m_CurrentFrame = 0;

				void CreateCommandBuffers(uint32_t _size);
				void RecreateSwapchain();

				// Create both VkSemaphore and VkFence
				void CreateSyncObject(uint32_t _size);
				void DestroySyncObject();
		};
	}
}