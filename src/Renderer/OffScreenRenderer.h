#pragma once

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanGraphicsPipeline.h"
#include "Renderer/VulkanDescriptor.h"

namespace Parfait
{
	namespace Graphics
	{
		class OffScreenRenderer
		{
			public:
				OffScreenRenderer(const VulkanContext& _vulkanContext, const VulkanDescriptor& _vulkanDescriptor);
				~OffScreenRenderer();

				bool UpdateScreenSize(int _sizeX, int _sizeY);
				void ReCreateFrameBuffer(int _sizeX, int _sizeY);

				const VkSampler& GetTextureSampler() const { return m_Sampler; }
				const VkImageView& GetTextureImageView() const { return m_Color.view; }
				const VkRenderPass& GetRenderPass() const { return m_RenderPass; }
				const VkFramebuffer& GetFramebuffer() const { return m_FrameBuffer; }
				const VulkanGraphicsPipeline& GetGraphicsPipeline() const { return *m_Pipeline; }

				const uint32_t& GetWidth() const { return m_Width; }
				const uint32_t& GetHeight() const { return m_Height; }

			private:
				const VulkanContext& m_VkContextRef;

				struct FrameBufferAttachment 
				{
					VkImage image;
					VkDeviceMemory memory;
					VkImageView view;
				};

				int32_t m_Width, m_Height;
				VkFramebuffer m_FrameBuffer;
				FrameBufferAttachment m_Color, m_Depth;
				VkRenderPass m_RenderPass;
				VkSampler m_Sampler;
				VkDescriptorImageInfo m_Descriptor;

				std::unique_ptr<VulkanGraphicsPipeline> m_Pipeline;

				void Initialize();

				void CreateColorFrameBuffer();
				void CreateDepthFrameBuffer();
				void CreateFrameBuffer();

				void DestroyOffScreenResize();
		};
	}
}