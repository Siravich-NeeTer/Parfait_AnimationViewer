#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <array>

#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanCommandPool.h"

namespace Parfait
{
	namespace Graphics
	{
		const int MAX_BONE_INFLUENCE = 4;

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 color;
			glm::vec2 uv;

			glm::ivec4 boneIDs;
			glm::vec4 weights;
			
			static VkVertexInputBindingDescription getBindingDescription() 
			{
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				return bindingDescription;
			}

			static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions() 
			{
				std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};

				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, position);

				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, normal);

				attributeDescriptions[2].binding = 0;
				attributeDescriptions[2].location = 2;
				attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[2].offset = offsetof(Vertex, color);

				attributeDescriptions[3].binding = 0;
				attributeDescriptions[3].location = 3;
				attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[3].offset = offsetof(Vertex, uv);

				attributeDescriptions[4].binding = 0;
				attributeDescriptions[4].location = 4;
				attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SINT;
				attributeDescriptions[4].offset = offsetof(Vertex, boneIDs);

				attributeDescriptions[5].binding = 0;
				attributeDescriptions[5].location = 5;
				attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
				attributeDescriptions[5].offset = offsetof(Vertex, weights);

				return attributeDescriptions;
			}
		};
		struct UniformBufferObject
		{
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 projection;
			glm::mat4 finalBonesMatrices[100];
		};

		class VulkanBuffer
		{
			public:
				VulkanBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkDeviceSize _size);
				VulkanBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkDeviceSize _size, const void* _data);
				virtual ~VulkanBuffer();

				const VkBuffer& GetBuffer() const { return m_Buffer; }
				const VkDeviceMemory& GetDeviceMemory() const { return m_BufferMemory; }
				const VkDeviceSize& GetDeviceSize() const { return m_BufferSize; }

			protected:
				const VulkanContext& m_VulkanContextRef;
				const VulkanCommandPool& m_VulkanCommandPoolRef;

				VkBuffer m_Buffer;
				VkDeviceMemory m_BufferMemory;
				VkDeviceSize m_BufferSize;

		};
	}
}