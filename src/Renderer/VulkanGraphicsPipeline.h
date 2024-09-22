#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>
#include <filesystem>

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanShader.h"
#include "Renderer/VulkanRenderPass.h"
#include "Renderer/VulkanDescriptor.h"
#include "Renderer/Buffers/VulkanBuffer.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanGraphicsPipeline
		{
			public:
				VulkanGraphicsPipeline(const VulkanContext& _vulkanContext, const std::vector<std::filesystem::path>& _shaderPaths);
				VulkanGraphicsPipeline(const VulkanContext& _vulkanContext, const VulkanRenderPass& _vulkanRenderPass, const VulkanDescriptor& _vulkanDescriptor, const std::vector<std::filesystem::path>& _shaderPaths);
				~VulkanGraphicsPipeline();

				const VkPipeline GetPipeline() const { return m_GraphicsPipeline; }
				const VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

			private:
				std::vector<std::filesystem::path> m_ShaderPaths;

				std::vector<VkShaderModule> m_ShaderModules;
				std::vector<VkPipelineShaderStageCreateInfo> m_PipelineShaderStages;

				VkPipeline m_GraphicsPipeline;
				VkPipelineLayout m_PipelineLayout;

				const VulkanContext& m_VulkanContextRef;

				void CreateGraphicsPipeline(const VkRenderPass& _renderPass, const VulkanDescriptor& _vulkanDescriptor);
				void CreateShaderModule();
		};
	}
}