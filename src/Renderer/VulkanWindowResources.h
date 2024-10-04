#pragma once

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include <chrono>
#include <windows.h>
#include <string>

#include "Core/Input.h"
#include "Core/Camera.h"
#include "Core/Model.h"

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanSurfaceSwapchain.h"
#include "Renderer/VulkanGraphicsPipeline.h"
#include "Renderer/VulkanRenderPass.h"
#include "Renderer/VulkanFramebuffer.h"
#include "Renderer/VulkanCommandPool.h"
#include "Renderer/VulkanCommandBuffer.h"
#include "Renderer/VulkanDescriptor.h"

#include "Renderer/OffScreenRenderer.h"

#include "Renderer/Buffers/VulkanVertexBuffer.h"
#include "Renderer/Buffers/VulkanIndexBuffer.h"
#include "Renderer/Buffers/VulkanUniformBuffer.h"

#include "Renderer/VulkanTexture.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanWindowResources
		{
			public:
				VulkanWindowResources(const VulkanContext& _vulkanContext, GLFWwindow* _window);
				~VulkanWindowResources();

				void Update(float dt);
				void Draw();
				void UpdateUniform(uint32_t _currentFrame);

				void BeginRenderPass(const VulkanCommandBuffer& _VkCommandBuffer, uint32_t _imageIndex);
				void EndRenderPass(const VulkanCommandBuffer& _VkCommandBuffer);

			private:
				const VulkanContext& m_VkContextRef;
				GLFWwindow* m_WindowRef;

				bool isCameraMove = false;
				Camera m_Camera;

				std::unique_ptr<VulkanSurfaceSwapchain> m_SurfaceSwapchain;
				std::unique_ptr<VulkanRenderPass> m_RenderPass;
				std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipeline;
				std::unique_ptr<VulkanFramebuffer> m_Framebuffers;
				std::unique_ptr<VulkanCommandPool> m_CommandPool;
				std::vector<std::unique_ptr<VulkanCommandBuffer>> m_CommandBuffers;
				std::unique_ptr<VulkanDescriptor> m_Descriptor;

				std::vector<VkSemaphore> m_PresentSemaphores;
				std::vector<VkSemaphore> m_RenderSemaphores;
				std::vector<VkFence> m_InflightFence;

				std::vector<std::unique_ptr<VulkanUniformBuffer<UniformBufferObject>>> m_UniformBuffers;
				
				std::vector<std::unique_ptr<VulkanTexture>> m_Textures;
				std::unique_ptr<VulkanTexture> m_DebugTexture;

				// Depth Buffer
				VkImage m_DepthImage;
				VkDeviceMemory m_DepthImageMemory;
				VkImageView m_DepthImageView;

				Model m_Model;

				std::unique_ptr<OffScreenRenderer> m_OffscreenRenderer;
				VkDescriptorSet m_ImGuiDescriptorSet;
				VkDescriptorPool m_ImGuiPool;

				int m_CurrentFrame = 0;
				bool m_IsFramebufferResize = false;

				void CreateCommandBuffers(uint32_t _size);
				void CreateSyncObject(uint32_t _size);
				void CreateDepthResources();
				void CreateImGui();

				void RecreateSwapchain();

				void DestroySyncObject();
				void DestroyDepthResources();

				// Window Events
				void BindWindowEvents();
				static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
		};
	}
}