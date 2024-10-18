#pragma once

#include <filesystem>

#include <stb_image.h>

#include "Renderer/VulkanContext.h"
#include "Renderer/Buffers/VulkanBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanTexture
		{
			public:
				VulkanTexture(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool);
				VulkanTexture(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, const std::filesystem::path& _texturePath);
				~VulkanTexture();

				void LoadTexture(const std::filesystem::path& _path);

				const VkImage& GetImage() const { return m_TextureImage; }
				const VkDeviceMemory& GetDeviceMemory() const { return m_TextureImageMemory; }
				const VkImageView& GetImageView() const { return m_TextureImageView; }
				const VkSampler& GetSampler() const { return m_TextureSampler; }
			
		private:
				const VulkanContext& m_VulkanContextRef;
				const VulkanCommandPool& m_VulkanCommandPoolRef;

				int m_Width, m_Height;
				int m_Channels;
				VkImage m_TextureImage;
				VkDeviceMemory m_TextureImageMemory;
				VkImageView m_TextureImageView;
				VkSampler m_TextureSampler;

				void CreateTextureImage(const std::filesystem::path& _path);
				void CreateTextureImageView();
				void CreateTextureSampler();

		};
	}
}