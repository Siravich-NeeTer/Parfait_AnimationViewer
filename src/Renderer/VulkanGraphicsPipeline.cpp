#include "VulkanGraphicsPipeline.h"

namespace Parfait
{
	namespace Graphics
	{
		VulkanGraphicsPipeline::VulkanGraphicsPipeline(const VulkanContext& _vulkanContext, const std::vector<std::filesystem::path>& _shaderPaths)
			: m_VulkanContextRef(_vulkanContext), m_ShaderPaths(_shaderPaths)
		{
		}
		VulkanGraphicsPipeline::VulkanGraphicsPipeline(const VulkanContext& _vulkanContext, const VulkanRenderPass& _vulkanRenderPass, const VulkanDescriptor& _vulkanDescriptor, const std::vector<std::filesystem::path>& _shaderPaths)
			: m_VulkanContextRef(_vulkanContext), m_ShaderPaths(_shaderPaths)
		{
			CreateGraphicsPipeline(_vulkanRenderPass.GetRenderPass(), _vulkanDescriptor);
		}

		VulkanGraphicsPipeline::VulkanGraphicsPipeline(const VulkanContext& _vulkanContext, const VkRenderPass& _vulkanRenderPass, const VulkanDescriptor& _vulkanDescriptor, const std::vector<std::filesystem::path>& _shaderPaths)
			: m_VulkanContextRef(_vulkanContext), m_ShaderPaths(_shaderPaths)
		{
			CreateGraphicsPipeline(_vulkanRenderPass, _vulkanDescriptor);
		}
		VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
		{
			vkDestroyPipeline(m_VulkanContextRef.GetLogicalDevice(), m_GraphicsPipeline, nullptr);
			vkDestroyPipelineLayout(m_VulkanContextRef.GetLogicalDevice(), m_PipelineLayout, nullptr);

			for (const VkShaderModule& shaderModule : m_ShaderModules)
			{
				vkDestroyShaderModule(m_VulkanContextRef.GetLogicalDevice(), shaderModule, nullptr);
			}
		}

		void VulkanGraphicsPipeline::CreateGraphicsPipeline(const VkRenderPass& _renderPass, const VulkanDescriptor& _vulkanDescriptor)
		{
			CreateShaderModule();

			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
			std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = Vertex::getAttributeDescriptions();

			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;

			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineDepthStencilStateCreateInfo depthStencil{};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.stencilTestEnable = VK_FALSE;

			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;

			std::vector<VkDynamicState> dynamicStates =
			{
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};
			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 1;
			pipelineLayoutInfo.pSetLayouts = &_vulkanDescriptor.GetDescriptorSetLayout();
			pipelineLayoutInfo.pushConstantRangeCount = 0;

			// TODO: Better Error Handler
			if (vkCreatePipelineLayout(m_VulkanContextRef.GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed to create pipeline layout!");
			}

			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = static_cast<uint32_t>(m_PipelineShaderStages.size());
			pipelineInfo.pStages = m_PipelineShaderStages.data();
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = &depthStencil;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;
			pipelineInfo.layout = m_PipelineLayout;
			pipelineInfo.renderPass = _renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

			// TODO: Better Error Handler
			if (vkCreateGraphicsPipelines(m_VulkanContextRef.GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create graphics pipeline!");
			}
		}
		void VulkanGraphicsPipeline::CreateShaderModule()
		{
			for (const std::filesystem::path& shaderPath : m_ShaderPaths)
			{
				std::vector<uint32_t> code = Shader::ReadFile(shaderPath);

				VkShaderModuleCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = code.size() * sizeof(uint32_t);
				createInfo.pCode = code.data();

				VkShaderModule shaderModule;
				if (vkCreateShaderModule(m_VulkanContextRef.GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
				{
					throw std::runtime_error("Failed to create shader module!");
				}
				m_ShaderModules.push_back(shaderModule);

				VkPipelineShaderStageCreateInfo shaderStageInfo{};
				shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStageInfo.stage = Shader::GetShaderStageFlag(shaderPath);
				shaderStageInfo.module = shaderModule;
				shaderStageInfo.pName = "main";
				m_PipelineShaderStages.push_back(shaderStageInfo);
			}
		}
	}
}