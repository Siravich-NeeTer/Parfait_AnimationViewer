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
				throw std::runtime_error("Failed to load texture image! (" + _path.string() + ")");
			}


			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			CreateBuffer(m_VulkanContextRef, m_VulkanCommandPoolRef, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
			vkUnmapMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory);

			stbi_image_free(pixels);

			CreateImage(m_VulkanContextRef, m_Width, m_Height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory);

			TransitionImageLayout(m_VulkanContextRef, m_VulkanCommandPoolRef, m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			CopyBufferToImage(m_VulkanContextRef, m_VulkanCommandPoolRef, stagingBuffer, m_TextureImage, static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height));
			TransitionImageLayout(m_VulkanContextRef, m_VulkanCommandPoolRef, m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			vkDestroyBuffer(m_VulkanContextRef.GetLogicalDevice(), stagingBuffer, nullptr);
			vkFreeMemory(m_VulkanContextRef.GetLogicalDevice(), stagingBufferMemory, nullptr);
		}
		void VulkanTexture::CreateTextureImageView()
		{
			m_TextureImageView = CreateImageView(m_VulkanContextRef, m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
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
	}
}