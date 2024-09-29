#pragma once

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include<assimp/quaternion.h>
#include<assimp/vector3.h>
#include<assimp/matrix4x4.h>
#include<glm/glm.hpp>
#include<glm/gtc/quaternion.hpp>

namespace Parfait
{
	namespace Graphics
	{
		class VulkanContext;
		class VulkanCommandPool;

		VkCommandBuffer BeginSingleTimeCommands(const VulkanContext& _vulkanContext, const VkCommandPool& _commandPool);
		void EndSingleTimeCommands(const VulkanContext& _vulkanContext, const VkCommandPool& _commandPool, VkCommandBuffer commandBuffer);
		void CopyBuffer(const VulkanContext& _vulkanContext, const VkCommandPool& _commandPool, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);
		VkImageView CreateImageView(const VulkanContext& _vulkanContext, VkImage image, VkFormat format, VkImageAspectFlags _aspectFlags);

		void CreateBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _buffer, VkDeviceMemory& _bufferMemory);
		void CopyBuffer(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);

		uint32_t FindMemoryType(const VulkanContext& _vulkanContext, uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void CreateImage(const VulkanContext& _vulkanContext, uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties, VkImage& _image, VkDeviceMemory& _imageMemory);
		void CopyBufferToImage(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height);
		void TransitionImageLayout(const VulkanContext& _vulkanContext, const VulkanCommandPool& _vulkanCommandPool, VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout);

		VkFormat FindSupportedFormat(const VulkanContext& _vulkanContext, const std::vector<VkFormat>& _formats, VkImageTiling _tiling, VkFormatFeatureFlags _features);
		VkFormat FindDepthFormat(const VulkanContext& _vulkanContext);
		bool HasStencilComponent(VkFormat _format);


		class AssimpGLMHelpers
		{
		public:

			static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
			{
				glm::mat4 to;
				//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
				to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
				to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
				to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
				to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
				return to;
			}

			static inline glm::vec3 GetGLMVec(const aiVector3D& vec)
			{
				return glm::vec3(vec.x, vec.y, vec.z);
			}

			static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
			{
				return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
			}
		};
	}
}