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
				VulkanGraphicsPipeline(const VulkanContext& _vulkanContext, const VkRenderPass& _vulkanRenderPass, const std::vector<VkDescriptorSetLayout>& _descriptorSetLayouts, const std::vector<std::filesystem::path>& _shaderPaths);
				VulkanGraphicsPipeline(const VulkanContext& _vulkanContext, const VkRenderPass& _vulkanRenderPass, std::vector<VkDescriptorSetLayout> _descriptorSetLayouts, std::vector<std::filesystem::path> _shaderPaths,
					VkVertexInputBindingDescription _inputBinding,
					std::vector<VkVertexInputAttributeDescription> _inputAttribute,
					uint32_t _pushConstantSize,
					VkPrimitiveTopology _topology,
					VkPolygonMode _polygonMode);
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

				void CreateGraphicsPipeline(const VkRenderPass& _renderPass, const std::vector<VkDescriptorSetLayout>& _descriptorSetLayouts);
				void CreateGraphicsPipeline(const VkRenderPass& _renderPass, const std::vector<VkDescriptorSetLayout>& _descriptorSetLayouts, 
					const VkVertexInputBindingDescription& _inputBinding,
					std::vector<VkVertexInputAttributeDescription> _inputAttribute,
					uint32_t _pushConstantSize,
					const VkPrimitiveTopology& _topology,
					const VkPolygonMode& _polygonMode);
				void CreateShaderModule();
		};
	}
}