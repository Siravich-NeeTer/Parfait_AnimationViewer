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
				VulkanSurfaceSwapchain m_VkSurfaceSwapchain;
				VulkanRenderPass m_VkRenderPass;
				std::unique_ptr<VulkanGraphicsPipeline> m_VkGraphicsPipeline;
				std::unique_ptr<VulkanFramebuffer> m_VkFramebuffer;
				VulkanCommandPool m_VkCommandPool;
				VulkanCommandBuffer m_VkCommandBuffer;

				VkSemaphore m_PresentSemaphore;
				VkSemaphore m_RenderSemaphore;
				VkFence m_Fence;

				// Create both VkSemaphore and VkFence
				void CreateSyncObject();
				void DestroySyncObject();
		};
	}
}