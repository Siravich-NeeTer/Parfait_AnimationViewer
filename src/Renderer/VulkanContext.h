#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vector>
#include <set>
#include <optional>

#include "Renderer/Utilities/VulkanValidation.h"
#include "Renderer/Utilities/VulkanUtilities.h"

namespace Parfait
{
	namespace Graphics
	{
		extern const std::string APPLICATION_NAME;
		extern const std::string ENGINE_NAME;
		extern const uint32_t VK_API_VERSION;

		extern const std::vector<const char*> deviceExtensions;
		extern const std::vector<const char*> validationLayers;

		static const int MAX_FRAMES_IN_FLIGHT = 2;

		#ifdef PARFAIT_DEBUG
			const bool enableValidationLayers = true;
		#else
			const bool enableValidationLayers = false;
		#endif

		class VulkanContext
		{
			public:
				VulkanContext();
				~VulkanContext();

				const VkInstance& GetInstance() const { return m_Instance; }
				const VkPhysicalDevice& GetPhysicalDevice() const { return m_PhysicalDevice; }
				const VkDevice& GetLogicalDevice() const { return m_Device; }

				const uint32_t GetQueueFamilySize() const { return m_QueueFamily.size(); }
				const uint32_t* GetQueueFamily() const { return m_QueueFamily.data(); }

				const VkQueue& GetGraphicsQueue() const { return m_GraphicsQueue; }
				const VkQueue& GetPresentQueue() const { return m_PresentQueue; }
				const VkQueue& GetComputeQueue() const { return m_ComputeQueue; }
				const VkQueue& GetTransferQueue() const { return m_TransferQueue; }

				const uint32_t GetGraphicsQueueFamily() const { return m_GraphicsFamily.value(); }
				const uint32_t GetPresentQueueFamily() const { return m_PresentFamily.value(); }
				const uint32_t GetComputeQueueFamily() const { return m_ComputeFamily.value(); }
				const uint32_t GetTransferQueueFamily() const { return m_TransferFamily.value(); }

			private:
				// Core Vulkan Components
				VkInstance m_Instance;
				VkPhysicalDevice m_PhysicalDevice;
				VkDevice m_Device;

				// Validation Component
				VkDebugUtilsMessengerEXT m_DebugMessenger;

				// Extensions components from both Physical & Logical Devices;
				// - PhysicalDevice sub-components
				VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
				VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;
				// - LogicalDevice sub-components
				std::vector<uint32_t> m_QueueFamily;
				VkQueue m_GraphicsQueue;
				std::optional<uint32_t> m_GraphicsFamily;
				VkQueue m_PresentQueue;
				std::optional<uint32_t> m_PresentFamily;
				VkQueue m_ComputeQueue;
				std::optional<uint32_t> m_ComputeFamily;
				VkQueue m_TransferQueue;
				std::optional<uint32_t> m_TransferFamily;

				void CreateInstance();
				void SetupDebugMessenger();
				void SetupPhysicalDevice();
				void SetupQueueFamily();
				void CreateLogicalDevice();

				bool IsValidationLayerSupport();
				bool IsDeviceExtensionSupport(const VkPhysicalDevice& device);
				bool IsPhysicalDeviceValid(const VkPhysicalDevice& device);
		};
	}
}