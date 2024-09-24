#pragma once

#include <vulkan/vulkan.hpp>

namespace Parfait
{
	namespace Graphics
	{
		class VulkanContext;
		class VulkanCommandPool;

		VkCommandBuffer BeginSingleTimeCommands(const VulkanContext& _vulkanContext, const VkCommandPool& _commandPool);
		void EndSingleTimeCommands(const VulkanContext& _vulkanContext, const VkCommandPool& _commandPool, VkCommandBuffer commandBuffer);
		void CopyBuffer(const VulkanContext& _vulkanContext, const VkCommandPool& _commandPool, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);
		VkImageView CreateImageView(const VulkanContext& _vulkanContext, VkImage image, VkFormat format, VkImageAspectFlags _aspectFlags);

		void CreateBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory);
		void CopyBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);

		uint32_t FindMemoryType(const VulkanContext& _vulkanContext, uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void CreateImage(const VulkanContext& _vulkanContext, uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties, VkImage& _image, VkDeviceMemory& _imageMemory);
		void CopyBufferToImage(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height);
		void TransitionImageLayout(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout);

		VkFormat FindSupportedFormat(const VulkanContext& _vulkanContext, const std::vector<VkFormat>& _formats, VkImageTiling _tiling, VkFormatFeatureFlags _features);
		VkFormat FindDepthFormat(const VulkanContext& _vulkanContext);
		bool HasStencilComponent(VkFormat _format);
	}
}