#include "OffScreenRenderer.h"

namespace Parfait
{
	namespace Graphics
	{
		OffScreenRenderer::OffScreenRenderer(const VulkanContext& _vulkanContext, const VulkanDescriptor& _vulkanDescriptor)
			: m_VkContextRef(_vulkanContext)
		{
			Initialize();
			m_Pipeline = std::make_unique<VulkanGraphicsPipeline>(_vulkanContext, m_RenderPass, _vulkanDescriptor, std::vector<std::filesystem::path>{"Shaders/mesh.vert", "Shaders/mesh.frag"});
		}
		OffScreenRenderer::OffScreenRenderer(const VulkanContext& _vulkanContext, const std::vector<VkDescriptorSetLayout>& _descriptorSetLayouts)
			: m_VkContextRef(_vulkanContext)
		{
			Initialize();
			m_Pipeline = std::make_unique<VulkanGraphicsPipeline>(_vulkanContext, m_RenderPass, _descriptorSetLayouts, std::vector<std::filesystem::path>{"Shaders/mesh.vert", "Shaders/mesh.frag"});
		}
		OffScreenRenderer::~OffScreenRenderer()
		{
			DestroyOffScreenResize();

			vkDestroySampler(m_VkContextRef.GetLogicalDevice(), m_Sampler, nullptr);
			vkDestroyRenderPass(m_VkContextRef.GetLogicalDevice(), m_RenderPass, nullptr);
		}

		bool OffScreenRenderer::UpdateScreenSize(int _sizeX, int _sizeY)
		{
			if (m_Width != _sizeX || m_Height != _sizeY)
			{
				return true;
			}
			return false;
		}

		void OffScreenRenderer::Initialize()
		{
			const int FB_SIZE_X = 512;
			const int FB_SIZE_Y = 512;
			const VkFormat FB_COLOR_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;

			m_Width = FB_SIZE_X;
			m_Height = FB_SIZE_Y;

			// Find a suitable depth format
			VkFormat fbDepthFormat = FindDepthFormat(m_VkContextRef);

			CreateColorFrameBuffer();
			CreateDepthFrameBuffer();

			// Create sampler to sample from the attachment in the fragment shader
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.maxAnisotropy = 1.0f;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV = samplerInfo.addressModeU;
			samplerInfo.addressModeW = samplerInfo.addressModeU;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.maxAnisotropy = 1.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 1.0f;
			samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			vkCreateSampler(m_VkContextRef.GetLogicalDevice(), &samplerInfo, nullptr, &m_Sampler);

			// Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering

			std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
			// Color attachment
			attchmentDescriptions[0].format = FB_COLOR_FORMAT;
			attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			// Depth attachment
			attchmentDescriptions[1].format = fbDepthFormat;
			attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

			VkSubpassDescription subpassDescription = {};
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &colorReference;
			subpassDescription.pDepthStencilAttachment = &depthReference;

			// Use subpass dependencies for layout transitions
			std::array<VkSubpassDependency, 2> dependencies;

			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_NONE_KHR;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			// Create the actual renderpass
			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
			renderPassInfo.pAttachments = attchmentDescriptions.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpassDescription;
			renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
			renderPassInfo.pDependencies = dependencies.data();

			vkCreateRenderPass(m_VkContextRef.GetLogicalDevice(), &renderPassInfo, nullptr, &m_RenderPass);

			CreateFrameBuffer();

			// Fill a descriptor for later use in a descriptor set
			m_Descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			m_Descriptor.imageView = m_Color.view;
			m_Descriptor.sampler = m_Sampler;
		}

		void OffScreenRenderer::CreateColorFrameBuffer()
		{
			// Color attachment
			VkImageCreateInfo image{};
			image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image.imageType = VK_IMAGE_TYPE_2D;
			image.format = VK_FORMAT_R8G8B8A8_UNORM;
			image.extent.width = m_Width;
			image.extent.height = m_Height;
			image.extent.depth = 1;
			image.mipLevels = 1;
			image.arrayLayers = 1;
			image.samples = VK_SAMPLE_COUNT_1_BIT;
			image.tiling = VK_IMAGE_TILING_OPTIMAL;
			// We will sample directly from the color attachment
			image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

			VkMemoryAllocateInfo memAlloc{};
			memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			VkMemoryRequirements memReqs;

			vkCreateImage(m_VkContextRef.GetLogicalDevice(), &image, nullptr, &m_Color.image);
			vkGetImageMemoryRequirements(m_VkContextRef.GetLogicalDevice(), m_Color.image, &memReqs);
			memAlloc.allocationSize = memReqs.size;
			memAlloc.memoryTypeIndex = FindMemoryType(m_VkContextRef, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			vkAllocateMemory(m_VkContextRef.GetLogicalDevice(), &memAlloc, nullptr, &m_Color.memory);
			vkBindImageMemory(m_VkContextRef.GetLogicalDevice(), m_Color.image, m_Color.memory, 0);

			VkImageViewCreateInfo colorImageView{};
			colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorImageView.format = VK_FORMAT_R8G8B8A8_UNORM;
			colorImageView.subresourceRange = {};
			colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorImageView.subresourceRange.baseMipLevel = 0;
			colorImageView.subresourceRange.levelCount = 1;
			colorImageView.subresourceRange.baseArrayLayer = 0;
			colorImageView.subresourceRange.layerCount = 1;
			colorImageView.image = m_Color.image;
			vkCreateImageView(m_VkContextRef.GetLogicalDevice(), &colorImageView, nullptr, &m_Color.view);

		}
		void OffScreenRenderer::CreateDepthFrameBuffer()
		{
			// Find a suitable depth format
			VkFormat fbDepthFormat = FindDepthFormat(m_VkContextRef);

			// Depth stencil attachment
			VkImageCreateInfo image{};
			image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image.imageType = VK_IMAGE_TYPE_2D;
			image.format = VK_FORMAT_R8G8B8A8_UNORM;
			image.extent.width = m_Width;
			image.extent.height = m_Height;
			image.extent.depth = 1;
			image.mipLevels = 1;
			image.arrayLayers = 1;
			image.samples = VK_SAMPLE_COUNT_1_BIT;
			image.tiling = VK_IMAGE_TILING_OPTIMAL;
			// We will sample directly from the color attachment
			image.format = fbDepthFormat;
			image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

			VkMemoryAllocateInfo memAlloc{};
			memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			VkMemoryRequirements memReqs;

			vkCreateImage(m_VkContextRef.GetLogicalDevice(), &image, nullptr, &m_Depth.image);
			vkGetImageMemoryRequirements(m_VkContextRef.GetLogicalDevice(), m_Depth.image, &memReqs);
			memAlloc.allocationSize = memReqs.size;
			memAlloc.memoryTypeIndex = FindMemoryType(m_VkContextRef, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			vkAllocateMemory(m_VkContextRef.GetLogicalDevice(), &memAlloc, nullptr, &m_Depth.memory);
			vkBindImageMemory(m_VkContextRef.GetLogicalDevice(), m_Depth.image, m_Depth.memory, 0);

			VkImageViewCreateInfo depthStencilView{};
			depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			depthStencilView.format = fbDepthFormat;
			depthStencilView.flags = 0;
			depthStencilView.subresourceRange = {};
			depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (fbDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
				depthStencilView.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			depthStencilView.subresourceRange.baseMipLevel = 0;
			depthStencilView.subresourceRange.levelCount = 1;
			depthStencilView.subresourceRange.baseArrayLayer = 0;
			depthStencilView.subresourceRange.layerCount = 1;
			depthStencilView.image = m_Depth.image;
			vkCreateImageView(m_VkContextRef.GetLogicalDevice(), &depthStencilView, nullptr, &m_Depth.view);

		}
		void OffScreenRenderer::CreateFrameBuffer()
		{
			VkImageView attachments[2];
			attachments[0] = m_Color.view;
			attachments[1] = m_Depth.view;

			VkFramebufferCreateInfo fbufCreateInfo{};
			fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbufCreateInfo.renderPass = m_RenderPass;
			fbufCreateInfo.attachmentCount = 2;
			fbufCreateInfo.pAttachments = attachments;
			fbufCreateInfo.width = m_Width;
			fbufCreateInfo.height = m_Height;
			fbufCreateInfo.layers = 1;

			vkCreateFramebuffer(m_VkContextRef.GetLogicalDevice(), &fbufCreateInfo, nullptr, &m_FrameBuffer);
		}

		void OffScreenRenderer::ReCreateFrameBuffer(int _sizeX, int _sizeY)
		{
			m_Width = _sizeX;
			m_Height = _sizeY;

			DestroyOffScreenResize();

			CreateColorFrameBuffer();
			CreateDepthFrameBuffer();
			CreateFrameBuffer();
		}

		void OffScreenRenderer::DestroyOffScreenResize()
		{
			vkDestroyImageView(m_VkContextRef.GetLogicalDevice(), m_Color.view, nullptr);
			vkDestroyImage(m_VkContextRef.GetLogicalDevice(), m_Color.image, nullptr);
			vkFreeMemory(m_VkContextRef.GetLogicalDevice(), m_Color.memory, nullptr);

			vkDestroyImageView(m_VkContextRef.GetLogicalDevice(), m_Depth.view, nullptr);
			vkDestroyImage(m_VkContextRef.GetLogicalDevice(), m_Depth.image, nullptr);
			vkFreeMemory(m_VkContextRef.GetLogicalDevice(), m_Depth.memory, nullptr);

			vkDestroyFramebuffer(m_VkContextRef.GetLogicalDevice(), m_FrameBuffer, nullptr);
		}
	}
}