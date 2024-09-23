#include "VulkanTexture.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanTexture::VulkanTexture(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool)
			: m_VulkanContextRef(_vulkanContext), m_VulkanCommandPoolRef(_vulkanCommandPool)
		{
		}
		VulkanTexture::VulkanTexture(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, const std::filesystem::path& _texturePath)
			: m_VulkanContextRef(_vulkanContext), m_VulkanCommandPoolRef(_vulkanCommandPool)
		{
			CreateTextureImage(_texturePath);
			CreateTextureImageView();
			CreateTextureSampler();
		}
		VulkanTexture::~VulkanTexture()
		{
			vkDestroySampler(m_VulkanContextRef.GetLogicalDevice(), m_TextureSampler, nullptr);
			vkDestroyImageView(m_VulkanContextRef.GetLogicalDevice(), m_TextureImageView, nullptr);

			vkDestroyImage(m_VulkanContextRef.GetLogicalDevice(), m_TextureImage, nullptr);
			vkFreeMemory(m_VulkanContextRef.GetLogicalDevice(), m_TextureImageMemory, nullptr);
		}

		void VulkanTexture::LoadTexture(const std::filesystem::path& _path)
		{
			CreateTextureImage(_path);
			CreateTextureImageView();
			CreateTextureSampler();
		}

		void VulkanTexture::CreateTextureImage(const std::filesystem::path& _path)
		{
			stbi_uc* pixels = stbi_load(_path.string().c_str(), &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha);
			VkDeviceSize imageSize = m_Width * m_Height * 4;

			if (!pixels) 
			{
				throw std::runtime_error("Failed to load texture image!");
			}


			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			VulkanBuffer::CreateBuffer(m_VulkanContextRef, m_VulkanCommandPoolRef, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
			vkUnmapMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory);

			stbi_image_free(pixels);

			CreateImage(m_VulkanContextRef, m_Width, m_Height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory);

			TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			CopyBufferToImage(stagingBuffer, m_TextureImage, static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height));
			TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			vkDestroyBuffer(m_VulkanContextRef.GetLogicalDevice(), stagingBuffer, nullptr);
			vkFreeMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory, nullptr);
		}
		void VulkanTexture::CreateTextureImageView()
		{
			m_TextureImageView = CreateImageView(m_VulkanContextRef, m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB);
		}
		void VulkanTexture::CreateTextureSampler()
		{
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; 
			samplerInfo.anisotropyEnable = VK_TRUE;

			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(m_VulkanContextRef.GetPhysicalDevice(), &properties);
			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 0.0f;

			if (vkCreateSampler(m_VulkanContextRef.GetLogicalDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to create texture sampler!");
			}
		}

		void VulkanTexture::CreateImage(const VulkanContext& _vulkanContext, uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties, VkImage& _image, VkDeviceMemory& _imageMemory)
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
			allocInfo.memoryTypeIndex = VulkanBuffer::FindMemoryType(_vulkanContext, memRequirements.memoryTypeBits, _properties);

			if (vkAllocateMemory(_vulkanContext.GetLogicalDevice(), &allocInfo, nullptr, &_imageMemory) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to allocate image memory!");
			}

			vkBindImageMemory(_vulkanContext.GetLogicalDevice(), _image, _imageMemory, 0);
		}
		void VulkanTexture::CopyBufferToImage(VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height) 
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommands(m_VulkanContextRef, m_VulkanCommandPoolRef.GetCommandPool());

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

			EndSingleTimeCommands(m_VulkanContextRef, m_VulkanCommandPoolRef.GetCommandPool(), commandBuffer);
		}
		void VulkanTexture::TransitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout)
		{
			VkCommandBuffer commandBuffer = BeginSingleTimeCommands(m_VulkanContextRef, m_VulkanCommandPoolRef.GetCommandPool());

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

			EndSingleTimeCommands(m_VulkanContextRef, m_VulkanCommandPoolRef.GetCommandPool(), commandBuffer);
		}
	}
}