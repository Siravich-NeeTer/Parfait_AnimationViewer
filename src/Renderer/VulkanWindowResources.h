#pragma once

#include <GLFW/glfw3.h>

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanSurfaceSwapchain.h"
#include "Renderer/VulkanGraphicsPipeline.h"
#include "Renderer/VulkanRenderPass.h"
#include "Renderer/VulkanFramebuffer.h"
#include "Renderer/VulkanCommandPool.h"
#include "Renderer/VulkanCommandBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		const int MAX_FRAMES_IN_FLIGHT = 2;

		class VulkanWindowResources
		{
			public:
				VulkanWindowResources(const VulkanContext& _vulkanContext, GLFWwindow* _window);
				~VulkanWindowResources();

				void Update();
				void Draw();

				void BeginRenderPass(const VulkanCommandBuffer& _VkCommandBuffer, uint32_t _imageIndex);
				void EndRenderPass(const VulkanCommandBuffer& _VkCommandBuffer);

			private:
				const VulkanContext& m_VkContextRef;
				GLFWwindow* m_WindowRef;

				std::unique_ptr<VulkanSurfaceSwapchain> m_SurfaceSwapchain;
				std::unique_ptr<VulkanRenderPass> m_RenderPass;
				std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipeline;
				std::unique_ptr<VulkanFramebuffer> m_Framebuffers;
				std::unique_ptr<VulkanCommandPool> m_CommandPool;
				std::vector<std::unique_ptr<VulkanCommandBuffer>> m_CommandBuffers;

				std::vector<VkSemaphore> m_PresentSemaphores;
				std::vector<VkSemaphore> m_RenderSemaphores;
				std::vector<VkFence> m_InflightFence;

				int m_CurrentFrame = 0;
				bool m_IsFramebufferResize = false;

				void CreateCommandBuffers(uint32_t _size);
				void CreateSyncObject(uint32_t _size);

				void RecreateSwapchain();

				void DestroySyncObject();

				// Window Events
				void BindWindowEvents();
				static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
		};
	}
}