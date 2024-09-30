#pragma once

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include <chrono>
#include <windows.h>
#include <string>

#include "Core/Input.h"
#include "Core/Camera.h"

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanSurfaceSwapchain.h"
#include "Renderer/VulkanGraphicsPipeline.h"
#include "Renderer/VulkanRenderPass.h"
#include "Renderer/VulkanFramebuffer.h"
#include "Renderer/VulkanCommandPool.h"
#include "Renderer/VulkanCommandBuffer.h"
#include "Renderer/VulkanDescriptor.h"

#include "Renderer/Buffers/VulkanVertexBuffer.h"
#include "Renderer/Buffers/VulkanIndexBuffer.h"
#include "Renderer/Buffers/VulkanUniformBuffer.h"

#include "Renderer/VulkanTexture.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanWindowResources
		{
			public:
				VulkanWindowResources(const VulkanContext& _vulkanContext, GLFWwindow* _window);
				~VulkanWindowResources();

				void Update(float dt);
				void Draw();
				void UpdateUniform(uint32_t _currentFrame);

				void LoadModel(const std::filesystem::path& _path);
				void ProcessNode(aiNode* node, const aiScene* scene);
				void ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4 _transform);

				void BeginRenderPass(const VulkanCommandBuffer& _VkCommandBuffer, uint32_t _imageIndex);
				void EndRenderPass(const VulkanCommandBuffer& _VkCommandBuffer);

			private:
				const VulkanContext& m_VkContextRef;
				GLFWwindow* m_WindowRef;

				bool isCameraMove = false;
				Camera m_Camera;

				std::unique_ptr<VulkanSurfaceSwapchain> m_SurfaceSwapchain;
				std::unique_ptr<VulkanRenderPass> m_RenderPass;
				std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipeline;
				std::unique_ptr<VulkanFramebuffer> m_Framebuffers;
				std::unique_ptr<VulkanCommandPool> m_CommandPool;
				std::vector<std::unique_ptr<VulkanCommandBuffer>> m_CommandBuffers;
				std::unique_ptr<VulkanDescriptor> m_Descriptor;

				std::vector<VkSemaphore> m_PresentSemaphores;
				std::vector<VkSemaphore> m_RenderSemaphores;
				std::vector<VkFence> m_InflightFence;

				std::unique_ptr<VulkanVertexBuffer<Vertex>> m_VertexBuffer;
				std::unique_ptr<VulkanIndexBuffer> m_IndexBuffer;
				std::vector<std::unique_ptr<VulkanUniformBuffer<UniformBufferObject>>> m_UniformBuffers;
				
				std::vector<std::unique_ptr<VulkanTexture>> m_Textures;
				std::unique_ptr<VulkanTexture> m_DebugTexture;

				// Depth Buffer
				VkImage m_DepthImage;
				VkDeviceMemory m_DepthImageMemory;
				VkImageView m_DepthImageView;

				std::vector<Vertex> m_Vertices;
				std::vector<uint32_t> m_Indices;

				VkDescriptorPool m_ImGuiPool;

				//TODO: TEMP
				// Framebuffer for offscreen rendering
				ImVec2 offscreenSize;
				VkDescriptorSet ds;
				std::unique_ptr<VulkanGraphicsPipeline> offscrenPipeline;
				struct FrameBufferAttachment {
					VkImage image;
					VkDeviceMemory mem;
					VkImageView view;
				};
				struct OffscreenPass {
					int32_t width, height;
					VkFramebuffer frameBuffer;
					FrameBufferAttachment color, depth;
					VkRenderPass renderPass;
					VkSampler sampler;
					VkDescriptorImageInfo descriptor;
				} offscreenPass{};
				VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat)
				{
					// Since all depth formats may be optional, we need to find a suitable depth format to use
					// Start with the highest precision packed format
					std::vector<VkFormat> formatList = {
						VK_FORMAT_D32_SFLOAT_S8_UINT,
						VK_FORMAT_D32_SFLOAT,
						VK_FORMAT_D24_UNORM_S8_UINT,
						VK_FORMAT_D16_UNORM_S8_UINT,
						VK_FORMAT_D16_UNORM
					};

					for (auto& format : formatList)
					{
						VkFormatProperties formatProps;
						vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
						if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
						{
							*depthFormat = format;
							return true;
						}
					}

					return false;
				}
				void prepareOffscreen()
				{
					const int FB_SIZE_X = 512;
					const int FB_SIZE_Y = 512;
					const VkFormat FB_COLOR_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;

					offscreenPass.width = FB_SIZE_X;
					offscreenPass.height = FB_SIZE_Y;

					// Find a suitable depth format
					VkFormat fbDepthFormat;
					VkBool32 validDepthFormat = getSupportedDepthFormat(m_VkContextRef.GetPhysicalDevice(), &fbDepthFormat);

					// Color attachment
					VkImageCreateInfo image{};
					image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
					image.imageType = VK_IMAGE_TYPE_2D;
					image.format = FB_COLOR_FORMAT;
					image.extent.width = offscreenPass.width;
					image.extent.height = offscreenPass.height;
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

					vkCreateImage(m_VkContextRef.GetLogicalDevice(), &image, nullptr, &offscreenPass.color.image);
					vkGetImageMemoryRequirements(m_VkContextRef.GetLogicalDevice(), offscreenPass.color.image, &memReqs);
					memAlloc.allocationSize = memReqs.size;
					memAlloc.memoryTypeIndex = FindMemoryType(m_VkContextRef, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
					vkAllocateMemory(m_VkContextRef.GetLogicalDevice(), &memAlloc, nullptr, &offscreenPass.color.mem);
					vkBindImageMemory(m_VkContextRef.GetLogicalDevice(), offscreenPass.color.image, offscreenPass.color.mem, 0);

					VkImageViewCreateInfo colorImageView{};
					colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
					colorImageView.format = FB_COLOR_FORMAT;
					colorImageView.subresourceRange = {};
					colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					colorImageView.subresourceRange.baseMipLevel = 0;
					colorImageView.subresourceRange.levelCount = 1;
					colorImageView.subresourceRange.baseArrayLayer = 0;
					colorImageView.subresourceRange.layerCount = 1;
					colorImageView.image = offscreenPass.color.image;
					vkCreateImageView(m_VkContextRef.GetLogicalDevice(), &colorImageView, nullptr, &offscreenPass.color.view);

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
					vkCreateSampler(m_VkContextRef.GetLogicalDevice(), &samplerInfo, nullptr, &offscreenPass.sampler);

					// Depth stencil attachment
					image.format = fbDepthFormat;
					image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

					vkCreateImage(m_VkContextRef.GetLogicalDevice(), &image, nullptr, &offscreenPass.depth.image);
					vkGetImageMemoryRequirements(m_VkContextRef.GetLogicalDevice(), offscreenPass.depth.image, &memReqs);
					memAlloc.allocationSize = memReqs.size;
					memAlloc.memoryTypeIndex = FindMemoryType(m_VkContextRef, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
					vkAllocateMemory(m_VkContextRef.GetLogicalDevice(), &memAlloc, nullptr, &offscreenPass.depth.mem);
					vkBindImageMemory(m_VkContextRef.GetLogicalDevice(), offscreenPass.depth.image, offscreenPass.depth.mem, 0);

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
					depthStencilView.image = offscreenPass.depth.image;
					vkCreateImageView(m_VkContextRef.GetLogicalDevice(), &depthStencilView, nullptr, &offscreenPass.depth.view);

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

					vkCreateRenderPass(m_VkContextRef.GetLogicalDevice(), &renderPassInfo, nullptr, &offscreenPass.renderPass);

					VkImageView attachments[2];
					attachments[0] = offscreenPass.color.view;
					attachments[1] = offscreenPass.depth.view;

					VkFramebufferCreateInfo fbufCreateInfo{};
					fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
					fbufCreateInfo.renderPass = offscreenPass.renderPass;
					fbufCreateInfo.attachmentCount = 2;
					fbufCreateInfo.pAttachments = attachments;
					fbufCreateInfo.width = offscreenPass.width;
					fbufCreateInfo.height = offscreenPass.height;
					fbufCreateInfo.layers = 1;

					vkCreateFramebuffer(m_VkContextRef.GetLogicalDevice(), &fbufCreateInfo, nullptr, &offscreenPass.frameBuffer);

					// Fill a descriptor for later use in a descriptor set
					offscreenPass.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					offscreenPass.descriptor.imageView = offscreenPass.color.view;
					offscreenPass.descriptor.sampler = offscreenPass.sampler;
				}
				void destroyOffscreenResize()
				{
					vkDestroyImageView(m_VkContextRef.GetLogicalDevice(), offscreenPass.color.view, nullptr);
					vkDestroyImage(m_VkContextRef.GetLogicalDevice(), offscreenPass.color.image, nullptr);
					vkFreeMemory(m_VkContextRef.GetLogicalDevice(), offscreenPass.color.mem, nullptr);

					vkDestroyImageView(m_VkContextRef.GetLogicalDevice(), offscreenPass.depth.view, nullptr);
					vkDestroyImage(m_VkContextRef.GetLogicalDevice(), offscreenPass.depth.image, nullptr);
					vkFreeMemory(m_VkContextRef.GetLogicalDevice(), offscreenPass.depth.mem, nullptr);

					vkDestroyFramebuffer(m_VkContextRef.GetLogicalDevice(), offscreenPass.frameBuffer, nullptr);
				}
				void destroyOffscreen()
				{
					destroyOffscreenResize();
					vkDestroySampler(m_VkContextRef.GetLogicalDevice(), offscreenPass.sampler, nullptr);
					vkDestroyRenderPass(m_VkContextRef.GetLogicalDevice(), offscreenPass.renderPass, nullptr);
				}
				void recreateOffscreen()
				{
					vkDeviceWaitIdle(m_VkContextRef.GetLogicalDevice());
					destroyOffscreenResize();

					const int FB_SIZE_X = offscreenSize.x;
					const int FB_SIZE_Y = offscreenSize.y;
					const VkFormat FB_COLOR_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;

					offscreenPass.width = FB_SIZE_X;
					offscreenPass.height = FB_SIZE_Y;

					// Find a suitable depth format
					VkFormat fbDepthFormat;
					VkBool32 validDepthFormat = getSupportedDepthFormat(m_VkContextRef.GetPhysicalDevice(), &fbDepthFormat);

					// Color attachment
					VkImageCreateInfo image{};
					image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
					image.imageType = VK_IMAGE_TYPE_2D;
					image.format = FB_COLOR_FORMAT;
					image.extent.width = offscreenPass.width;
					image.extent.height = offscreenPass.height;
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

					vkCreateImage(m_VkContextRef.GetLogicalDevice(), &image, nullptr, &offscreenPass.color.image);
					vkGetImageMemoryRequirements(m_VkContextRef.GetLogicalDevice(), offscreenPass.color.image, &memReqs);
					memAlloc.allocationSize = memReqs.size;
					memAlloc.memoryTypeIndex = FindMemoryType(m_VkContextRef, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
					vkAllocateMemory(m_VkContextRef.GetLogicalDevice(), &memAlloc, nullptr, &offscreenPass.color.mem);
					vkBindImageMemory(m_VkContextRef.GetLogicalDevice(), offscreenPass.color.image, offscreenPass.color.mem, 0);

					VkImageViewCreateInfo colorImageView{};
					colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
					colorImageView.format = FB_COLOR_FORMAT;
					colorImageView.subresourceRange = {};
					colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					colorImageView.subresourceRange.baseMipLevel = 0;
					colorImageView.subresourceRange.levelCount = 1;
					colorImageView.subresourceRange.baseArrayLayer = 0;
					colorImageView.subresourceRange.layerCount = 1;
					colorImageView.image = offscreenPass.color.image;
					vkCreateImageView(m_VkContextRef.GetLogicalDevice(), &colorImageView, nullptr, &offscreenPass.color.view);

					// Depth stencil attachment
					image.format = fbDepthFormat;
					image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

					vkCreateImage(m_VkContextRef.GetLogicalDevice(), &image, nullptr, &offscreenPass.depth.image);
					vkGetImageMemoryRequirements(m_VkContextRef.GetLogicalDevice(), offscreenPass.depth.image, &memReqs);
					memAlloc.allocationSize = memReqs.size;
					memAlloc.memoryTypeIndex = FindMemoryType(m_VkContextRef, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
					vkAllocateMemory(m_VkContextRef.GetLogicalDevice(), &memAlloc, nullptr, &offscreenPass.depth.mem);
					vkBindImageMemory(m_VkContextRef.GetLogicalDevice(), offscreenPass.depth.image, offscreenPass.depth.mem, 0);

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
					depthStencilView.image = offscreenPass.depth.image;
					vkCreateImageView(m_VkContextRef.GetLogicalDevice(), &depthStencilView, nullptr, &offscreenPass.depth.view);

					VkImageView attachments[2];
					attachments[0] = offscreenPass.color.view;
					attachments[1] = offscreenPass.depth.view;

					VkFramebufferCreateInfo fbufCreateInfo{};
					fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
					fbufCreateInfo.renderPass = offscreenPass.renderPass;
					fbufCreateInfo.attachmentCount = 2;
					fbufCreateInfo.pAttachments = attachments;
					fbufCreateInfo.width = offscreenPass.width;
					fbufCreateInfo.height = offscreenPass.height;
					fbufCreateInfo.layers = 1;

					vkCreateFramebuffer(m_VkContextRef.GetLogicalDevice(), &fbufCreateInfo, nullptr, &offscreenPass.frameBuffer);

					// Fill a descriptor for later use in a descriptor set
					offscreenPass.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					offscreenPass.descriptor.imageView = offscreenPass.color.view;
					offscreenPass.descriptor.sampler = offscreenPass.sampler;

					ds = ImGui_ImplVulkan_AddTexture(offscreenPass.sampler, offscreenPass.color.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				}

				int m_CurrentFrame = 0;
				bool m_IsFramebufferResize = false;

				void CreateCommandBuffers(uint32_t _size);
				void CreateSyncObject(uint32_t _size);
				void CreateDepthResources();
				void CreateImGui();

				void RecreateSwapchain();

				void DestroySyncObject();
				void DestroyDepthResources();

				// Window Events
				void BindWindowEvents();
				static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
		};
	}
}