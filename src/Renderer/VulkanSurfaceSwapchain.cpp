#include "VulkanSurfaceSwapchain.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanSurfaceSwapchain::VulkanSurfaceSwapchain(const VulkanContext& _vulkanContext, GLFWwindow& _window)
			: m_VulkanContextRef(_vulkanContext),
			m_WindowRef(_window)
		{
			CreateSurface();
			CreateSwapchain();
			CreateImageViews();
		}
		VulkanSurfaceSwapchain::~VulkanSurfaceSwapchain()
		{
			DestroySwapchainImageViews();
			vkDestroySurfaceKHR(m_VulkanContextRef.GetInstance(), m_Surface, nullptr);
		}
		void VulkanSurfaceSwapchain::RecreateSwapchainImageViews()
		{
			DestroySwapchainImageViews();

			CreateSwapchain();
			CreateImageViews();
		}

		void VulkanSurfaceSwapchain::CreateSurface()
		{
			// TODO: Better Error Handler
			if (glfwCreateWindowSurface(m_VulkanContextRef.GetInstance(), &m_WindowRef, nullptr, &m_Surface) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create Window Surface!");
			}
		}
		void VulkanSurfaceSwapchain::CreateImageViews()
		{
			m_SwapchainImageViews.resize(m_SwapchainImages.size());

			for (size_t i = 0; i < m_SwapchainImages.size(); i++)
			{
				m_SwapchainImageViews[i] = CreateImageView(m_VulkanContextRef, m_SwapchainImages[i], m_SurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
			}
		}
		void VulkanSurfaceSwapchain::CreateSwapchain()
		{
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_VulkanContextRef.GetPhysicalDevice(), m_Surface, &m_SurfaceCapabilities);

			m_SurfaceFormat = ChooseSwapSurfaceFormat();
			m_PresentMode = ChooseSwapPresentMode();
			m_Extent = ChooseSwapExtent();

			uint32_t imageCount = m_SurfaceCapabilities.minImageCount + 1;
			if (m_SurfaceCapabilities.maxImageCount > 0 && imageCount > m_SurfaceCapabilities.maxImageCount)
			{
				imageCount = m_SurfaceCapabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = m_Surface;
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = m_SurfaceFormat.format;
			createInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
			createInfo.imageExtent = m_Extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			if (m_VulkanContextRef.GetQueueFamilySize() > 1)
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = m_VulkanContextRef.GetQueueFamilySize();
				createInfo.pQueueFamilyIndices = m_VulkanContextRef.GetQueueFamily();
			}
			else
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}
			createInfo.preTransform = m_SurfaceCapabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = m_PresentMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			if (vkCreateSwapchainKHR(m_VulkanContextRef.GetLogicalDevice(), &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create swap chain!");
			}

			vkGetSwapchainImagesKHR(m_VulkanContextRef.GetLogicalDevice(), m_Swapchain, &imageCount, nullptr);
			m_SwapchainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(m_VulkanContextRef.GetLogicalDevice(), m_Swapchain, &imageCount, m_SwapchainImages.data());
		}

		VkSurfaceFormatKHR VulkanSurfaceSwapchain::ChooseSwapSurfaceFormat()
		{
			uint32_t surfaceFormatCount = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(m_VulkanContextRef.GetPhysicalDevice(), m_Surface, &surfaceFormatCount, nullptr);
			std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(m_VulkanContextRef.GetPhysicalDevice(), m_Surface, &surfaceFormatCount, surfaceFormats.data());

			for (const VkSurfaceFormatKHR& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					return surfaceFormat;
				}
			}
			return surfaceFormats[0];
		}
		VkPresentModeKHR VulkanSurfaceSwapchain::ChooseSwapPresentMode()
		{
			uint32_t presentModeCount = 0;
			vkGetPhysicalDeviceSurfacePresentModesKHR(m_VulkanContextRef.GetPhysicalDevice(), m_Surface, &presentModeCount, nullptr);
			std::vector<VkPresentModeKHR> presentModes(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(m_VulkanContextRef.GetPhysicalDevice(), m_Surface, &presentModeCount, presentModes.data());

			for (const VkPresentModeKHR& presentMode : presentModes)
			{
				if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return presentMode;
				}
			}
			return VK_PRESENT_MODE_FIFO_KHR;
		}
		VkExtent2D VulkanSurfaceSwapchain::ChooseSwapExtent()
		{
			if (m_SurfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			{
				return m_SurfaceCapabilities.currentExtent;
			}
			else
			{
				int width, height;
				glfwGetFramebufferSize(&m_WindowRef, &width, &height);

				VkExtent2D actualExtent = {
					static_cast<uint32_t>(width),
					static_cast<uint32_t>(height)
				};

				actualExtent.width = std::clamp(actualExtent.width, m_SurfaceCapabilities.minImageExtent.width, m_SurfaceCapabilities.maxImageExtent.width);
				actualExtent.height = std::clamp(actualExtent.height, m_SurfaceCapabilities.minImageExtent.height, m_SurfaceCapabilities.maxImageExtent.height);

				return actualExtent;
			}
		}

		void VulkanSurfaceSwapchain::DestroySwapchainImageViews()
		{
			for (const VkImageView& imageView : m_SwapchainImageViews)
			{
				vkDestroyImageView(m_VulkanContextRef.GetLogicalDevice(), imageView, nullptr);
			}

			vkDestroySwapchainKHR(m_VulkanContextRef.GetLogicalDevice(), m_Swapchain, nullptr);
		}
	}
}