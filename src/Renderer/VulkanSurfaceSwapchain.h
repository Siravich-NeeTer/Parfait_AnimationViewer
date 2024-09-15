#pragma once

#include <cstdint>
#include <limits>
#include <algorithm>

#include "Renderer/VulkanContext.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanSurfaceSwapchain
		{
			public:
				VulkanSurfaceSwapchain(const VulkanContext& _vulkanContext, GLFWwindow& _window);
				~VulkanSurfaceSwapchain();

				void RecreateSwapchainImageViews();

				const VkSurfaceKHR& GetSurface() const { return m_Surface; }
				const VkSurfaceFormatKHR& GetSurfaceFormat() const { return m_SurfaceFormat; }
				const VkSurfaceCapabilitiesKHR& GetSurfaceCapabilities() const { return m_SurfaceCapabilities; }
				const VkPresentModeKHR& GetPresentMode() const { return m_PresentMode; }
				const VkExtent2D& GetExtent() const { return m_Extent; }
				const VkSwapchainKHR& GetSwapchain() const { return m_Swapchain; }
				const std::vector<VkImage>& GetSwapchainImages() const { return m_SwapchainImages; }
				const std::vector<VkImageView>& GetSwapchainImageViews() const { return m_SwapchainImageViews; }

			private:
				const VulkanContext& m_VulkanContextRef;
				GLFWwindow& m_WindowRef;

				VkSurfaceKHR m_Surface;
				VkSurfaceFormatKHR m_SurfaceFormat;
				VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;

				VkPresentModeKHR m_PresentMode;
				VkExtent2D m_Extent;

				VkSwapchainKHR m_Swapchain;
				std::vector<VkImage> m_SwapchainImages;
				std::vector<VkImageView> m_SwapchainImageViews;

				void CreateSurface();
				void CreateSwapchain();
				void CreateImageViews();
				VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

				VkSurfaceFormatKHR ChooseSwapSurfaceFormat();
				VkPresentModeKHR ChooseSwapPresentMode();
				VkExtent2D ChooseSwapExtent();

				void DestroySwapchainImageViews();
		};
	}
}