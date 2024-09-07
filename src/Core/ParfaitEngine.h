#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <exception>
#include <vector>
#include <set>
#include <map>
#include <optional>

#include "Renderer/VulkanUtility.h"
#include "Renderer/BufferObject.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif // !NDEBUG


class ParfaitEngine
{
	public:
		void Run();

	private:
		void InitVulkan();
		void InitWindow();
		void MainLoop();
		void DrawFrame();
		void CleanUp();
		void CleanUpSwapChain();

		GLFWwindow* m_Window;

		// Window Callbacks
		static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height)
		{
			auto app = reinterpret_cast<ParfaitEngine*>(glfwGetWindowUserPointer(window));
			app->m_FrameBufferResized = true;
		}

		// Vulkan Component
		// - Vulkan Variables
		VkInstance m_VkInstance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkSurfaceKHR m_Surface;

		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device;
		
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;
		
		VkSwapchainKHR m_Swapchain;
		std::vector<VkImage> m_SwapchainImages;
		VkFormat m_SwapchainImageFormat;
		VkExtent2D m_SwapchainExtent;
		std::vector<VkImageView> m_SwapchainImageViews;
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;
		
		VkRenderPass m_RenderPass;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;

		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		uint32_t m_CurrentFrame = 0;
		bool m_FrameBufferResized = false;

		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		// - Vulkan Functions
		// -- Create Core Functions
		void CreateVulkanInstance();
		void CreateSurface();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateVertexBuffer(); 
		void CreateIndexBuffer();
		void CreateCommandBuffer();
		void CreateSyncObjects();

		void RecreateSwapChain();

		// -- Getter Functions
		std::vector<const char*> GetRequiredExtensions();
		VkPhysicalDevice GetPhysicalDevice();
		
		// -- Checker Functions
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool IsDeviceExtensionSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		
		// -- Support Functions
		Parfait::vulkan::QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		Parfait::vulkan::SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		// -- Validation Layer Functions
		void SetupDebugMessenger();
		bool CheckValidationLayerSupport();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
		{
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			// Which message severity will call pfnUserCallback (Verbose, Warning, Error)
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			// Type of message callback get called
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = Parfait::vulkan::DebugMessengerCallback;
			// Optional (Will automatically be null from createInfo{})
			createInfo.pUserData = nullptr;
		}
};