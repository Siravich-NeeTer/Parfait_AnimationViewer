#include "VulkanContext.h"

namespace Parfait
{
	namespace Graphics
	{

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			{
				std::cerr << pCallbackData->pMessage << "\n";
			}
			return VK_FALSE;
		}
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
		{
			PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}
			else
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}
		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
		{
			// Pointer function of vkDestroyDebugUtilsMessengerEXT
			PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr)
				return func(instance, debugMessenger, pAllocator);
		}
		void SetupDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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
			createInfo.pfnUserCallback = DebugCallback;
			// Optional (Will automatically be null from createInfo{})
			createInfo.pUserData = nullptr;
		}

		VulkanContext::VulkanContext()
		{
			CreateInstance();
			SetupDebugMessenger();
			SetupPhysicalDevice();
			SetupQueueFamily();
			CreateLogicalDevice();
		}
		VulkanContext::~VulkanContext()
		{
			vkDestroyDevice(m_Device, nullptr);

			if (enableValidationLayers) 
			{
				DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
			}

			vkDestroyInstance(m_Instance, nullptr);
		}

		void VulkanContext::CreateInstance()
		{
			// TODO: Better Error Handler
			if (enableValidationLayers && !IsValidationLayerSupport())
			{
				throw std::runtime_error("Validation layers requested, but not available!");
			}

			VkApplicationInfo appInfo{};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = APPLICATION_NAME.c_str();
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = ENGINE_NAME.c_str();
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION;

			// Get GLFW Extensions
			uint32_t glfwExtensionsCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
			// - Add validation extension to extensions list
			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);
			if (enableValidationLayers)
			{
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}

			VkInstanceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;
			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();
			// Enable validation layers
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
			if (enableValidationLayers)
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();

				SetupDebugMessengerCreateInfo(debugCreateInfo);
				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
			}
			else
			{
				createInfo.enabledLayerCount = 0;
				createInfo.pNext = nullptr;
			}

			// TODO: Better Error Handler
			if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
				throw std::runtime_error("Failed to create instance!");
		}
		void VulkanContext::SetupDebugMessenger()
		{
			if (!enableValidationLayers)
			{
				return;
			}

			VkDebugUtilsMessengerCreateInfoEXT createInfo{};
			SetupDebugMessengerCreateInfo(createInfo);
			
			// TODO: Better Error Handler
			if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to set up debug messenger!");
			}
		}
		void VulkanContext::SetupPhysicalDevice()
		{
			uint32_t physicalDeviceCount = 0;

			vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr);
			std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
			vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, physicalDevices.data());

			// Choose appropriate physical device
			for (const VkPhysicalDevice& physicalDevice : physicalDevices)
			{
				if (IsPhysicalDeviceValid(physicalDevice))
				{
					m_PhysicalDevice = physicalDevice;

					vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties);
					vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_PhysicalDeviceFeatures);

					break;
				}
			}

			// TODO: Better Error Handler
			if (m_PhysicalDevice == VK_NULL_HANDLE)
			{
				throw std::runtime_error("Failed to find a suitable GPU!");
			}
		}
		void VulkanContext::SetupQueueFamily()
		{
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

			uint32_t i = 0;
			for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					m_GraphicsFamily = i;
				// TODO: Find better ways to get PresentFamily
				if (queueFamily.queueCount > 0)
					m_PresentFamily = i;
				if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
					m_ComputeFamily = i;
				if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
					m_TransferFamily = i;

				if (m_GraphicsFamily.has_value() && 
					m_PresentFamily.has_value() && 
					m_ComputeFamily.has_value() &&
					m_TransferFamily.has_value())
					break;

				i++;
			}
		}
		void VulkanContext::CreateLogicalDevice()
		{
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
			std::set<uint32_t> uniqueQueueFamilies = { m_GraphicsFamily.value(), m_PresentFamily.value(), m_ComputeFamily.value(), m_TransferFamily.value() };

			float queuePriority = 1.0f;
			for (uint32_t queueFamily : uniqueQueueFamilies)
			{
				VkDeviceQueueCreateInfo queueCreateInfo{};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			VkPhysicalDeviceFeatures deviceFeatures{};
			//deviceFeatures.samplerAnisotropy = VK_TRUE;
			//deviceFeatures.sampleRateShading = VK_TRUE;

			VkDeviceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.pQueueCreateInfos = queueCreateInfos.data();
			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pEnabledFeatures = &deviceFeatures;
			createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();
			// TODO: Validation Layers
			if (enableValidationLayers)
			{
				createInfo.ppEnabledLayerNames = validationLayers.data();
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			}
			else
			{
				createInfo.enabledLayerCount = 0;
			}

			// TODO: Better Error Handler
			if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create logical device!");
			}

			vkGetDeviceQueue(m_Device, m_GraphicsFamily.value(), 0, &m_GraphicsQueue);
			vkGetDeviceQueue(m_Device, m_PresentFamily.value(), 0, &m_PresentQueue);
			vkGetDeviceQueue(m_Device, m_ComputeFamily.value(), 0, &m_ComputeQueue);
			vkGetDeviceQueue(m_Device, m_TransferFamily.value(), 0, &m_TransferQueue);

			m_QueueFamily.assign(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());
		}

		bool VulkanContext::IsValidationLayerSupport()
		{
			uint32_t layerCount = 0;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			for (const char* layerName : validationLayers)
			{
				bool layerFound = false;
				for (const VkLayerProperties& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
					}
				}

				if (!layerFound)
				{
					return false;
				}
			}
			return true;
		}
		bool VulkanContext::IsDeviceExtensionSupport(const VkPhysicalDevice& device)
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
			for (const VkExtensionProperties& extension : availableExtensions) 
			{
				requiredExtensions.erase(extension.extensionName);
			}

			return requiredExtensions.empty();
		}
		bool VulkanContext::IsPhysicalDeviceValid(const VkPhysicalDevice& device)
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;
			VkPhysicalDeviceFeatures physicalDeviceFeatures;
			vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
			vkGetPhysicalDeviceFeatures(device, &physicalDeviceFeatures);
			
			return physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && 
				physicalDeviceFeatures.geometryShader &&
				IsDeviceExtensionSupport(device);
		}
	}
}