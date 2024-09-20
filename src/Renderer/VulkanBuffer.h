#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <array>

#include "Renderer/VulkanContext.h"

namespace Parfait
{
	namespace Graphics
	{
		struct Vertex
		{
			glm::vec2 pos;
			glm::vec3 color;
			
			static VkVertexInputBindingDescription getBindingDescription() 
			{
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				return bindingDescription;
			}

			static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() 
			{
				std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, pos);

				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, color);

				return attributeDescriptions;
			}
		};
		
		const std::vector<Vertex> vertices = {
			{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		class VulkanBuffer
		{
			public:
				VulkanBuffer(const VulkanContext& _vulkanContext);
				~VulkanBuffer();

				const VkBuffer& GetBuffer() const { return m_Buffer; }

			private:
				const VulkanContext& m_VulkanContextRef;

				VkBuffer m_Buffer;
				VkDeviceMemory m_BufferMemory;

				void CreateBuffer();

				uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		};
	}
}