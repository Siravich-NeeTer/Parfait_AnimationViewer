#include "VulkanUtilities.h"

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanCommandPool.h"

namespace Parfait
{
	namespace Graphics
	{
		VkCommandBuffer BeginSingleTimeCommands(const VulkanContext& _vulkanContext, const VkCommandPool& _commandPool)
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = _commandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(_vulkanContext.GetLogicalDevice(), &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			return commandBuffer;
		}
		void EndSingleTimeCommands(const VulkanContext& _vulkanContext, const VkCommandPool& _commandPool, VkCommandBuffer commandBuffer)
		{
			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(_vulkanContext.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(_vulkanContext.GetGraphicsQueue());

			vkFreeCommandBuffers(_vulkanContext.GetLogicalDevice(), _commandPool, 1, &commandBuffer);
		}
		void CopyBuffer(const VulkanContext& _vulkanContext, const VkCommandPool& _commandPool, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommands(_vulkanContext, _commandPool);

			VkBufferCopy copyRegion{};
			copyRegion.size = _size;
			vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);

			EndSingleTimeCommands(_vulkanContext, _commandPool, commandBuffer);
		}
		VkImageView CreateImageView(const VulkanContext& _vulkanContext, VkImage image, VkFormat format, VkImageAspectFlags _aspectFlags)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = _aspectFlags;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			VkImageView imageView;
			if (vkCreateImageView(_vulkanContext.GetLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create texture image view!");
			}

			return imageView;
		}

		void CreateBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory)
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = _size;
			bufferInfo.usage = _usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(_vulkanContext.GetLogicalDevice(), &bufferInfo, nullptr, &_buffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to create buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(_vulkanContext.GetLogicalDevice(), _buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(_vulkanContext, memRequirements.memoryTypeBits, _properties);

			if (vkAllocateMemory(_vulkanContext.GetLogicalDevice(), &allocInfo, nullptr, &_bufferMemory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate buffer memory!");
			}

			vkBindBufferMemory(_vulkanContext.GetLogicalDevice(), _buffer, _bufferMemory, 0);
		}
		void CopyBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = _vulkanCommandPool.GetCommandPool();
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(_vulkanContext.GetLogicalDevice(), &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);
			{
				VkBufferCopy copyRegion{};
				copyRegion.size = _size;
				vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);
			}
			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(_vulkanContext.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(_vulkanContext.GetGraphicsQueue());

			vkFreeCommandBuffers(_vulkanContext.GetLogicalDevice(), _vulkanCommandPool.GetCommandPool(), 1, &commandBuffer);
		}

		uint32_t FindMemoryType(const VulkanContext& _vulkanContext, uint32_t typeFilter, VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(_vulkanContext.GetPhysicalDevice(), &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}

			throw std::runtime_error("Failed to find suitable memory type!");
		}

		void CreateImage(const VulkanContext& _vulkanContext, uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties, VkImage& _image, VkDeviceMemory& _imageMemory)
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = _width;
			imageInfo.extent.height = _height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = _format;
			imageInfo.tiling = _tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = _usage;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateImage(_vulkanContext.GetLogicalDevice(), &imageInfo, nullptr, &_image) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create image!");
			}

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(_vulkanContext.GetLogicalDevice(), _image, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(_vulkanContext, memRequirements.memoryTypeBits, _properties);

			if (vkAllocateMemory(_vulkanContext.GetLogicalDevice(), &allocInfo, nullptr, &_imageMemory) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to allocate image memory!");
			}

			vkBindImageMemory(_vulkanContext.GetLogicalDevice(), _image, _imageMemory, 0);
		}
		void CopyBufferToImage(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height)
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommands(_vulkanContext, _vulkanCommandPool.GetCommandPool());

			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				_width,
				_height,
				1
			};

			vkCmdCopyBufferToImage(commandBuffer, _buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			EndSingleTimeCommands(_vulkanContext, _vulkanCommandPool.GetCommandPool(), commandBuffer);
		}
		void TransitionImageLayout(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout)
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommands(_vulkanContext, _vulkanCommandPool.GetCommandPool());

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = _oldLayout;
			barrier.newLayout = _newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = _image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else
			{
				throw std::invalid_argument("Unsupported layout transition!");
			}

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			EndSingleTimeCommands(_vulkanContext, _vulkanCommandPool.GetCommandPool(), commandBuffer);
		}

		VkFormat FindSupportedFormat(const VulkanContext& _vulkanContext, const std::vector<VkFormat>& _formats, VkImageTiling _tiling, VkFormatFeatureFlags _features)
		{
			for (const VkFormat& format : _formats)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(_vulkanContext.GetPhysicalDevice(), format, &props);

				if (_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & _features) == _features)
				{
					return format;
				}
				else if (_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & _features) == _features)
				{
					return format;
				}
			}

			throw std::runtime_error("Failed to find supported format!");
		}
		VkFormat FindDepthFormat(const VulkanContext& _vulkanContext)
		{
			return FindSupportedFormat(_vulkanContext, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		}
		bool HasStencilComponent(VkFormat _format)
		{
			return _format == VK_FORMAT_D32_SFLOAT_S8_UINT || _format == VK_FORMAT_D24_UNORM_S8_UINT;
		}
	}
}