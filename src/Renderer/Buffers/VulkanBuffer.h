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
		struct Vertex
		{
			glm::vec2 pos;
			glm::vec3 color;
			glm::vec2 texCoord;
			
			static VkVertexInputBindingDescription getBindingDescription() 
			{
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				return bindingDescription;
			}

			static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() 
			{
				std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, pos);

				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, color);

				attributeDescriptions[2].binding = 0;
				attributeDescriptions[2].location = 2;
				attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

				return attributeDescriptions;
			}
		};
		struct UniformBufferObject
		{
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 projection;
		};
		
		extern const std::vector<Vertex> vertices;
		extern const std::vector<uint16_t> indices;

		class VulkanBuffer
		{
			public:
				VulkanBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkDeviceSize _size);
				VulkanBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkDeviceSize _size, const void* _data);
				virtual ~VulkanBuffer();

				const VkBuffer& GetBuffer() const { return m_Buffer; }
				const VkDeviceMemory& GetDeviceMemory() const { return m_BufferMemory; }
				const VkDeviceSize& GetDeviceSize() const { return m_BufferSize; }

				static void CreateBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory);
				static void CopyBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);
				static uint32_t FindMemoryType(const VulkanContext& _vulkanContext, uint32_t typeFilter, VkMemoryPropertyFlags properties);

			protected:
				const VulkanContext& m_VulkanContextRef;
				const VulkanCommandPool& m_VulkanCommandPoolRef;

				VkBuffer m_Buffer;
				VkDeviceMemory m_BufferMemory;
				VkDeviceSize m_BufferSize;

		};
	}
}