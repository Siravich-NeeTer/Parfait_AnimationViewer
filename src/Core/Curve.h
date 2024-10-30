#pragma once

#include <vector>
#include <string>

#include "Renderer/Buffers/VulkanVertexBuffer.h"

#include "Core/Object.h"

namespace Parfait
{
	struct PointVertex
	{
		glm::vec3 position;
		glm::vec3 color;
	};

	class Curve
	{
		public:
			Curve(const Graphics::VulkanContext& _vulkanContext, const Graphics::VulkanCommandPool& _vulkanCommandPool);
			void AddPoint(const glm::vec3& _newPosition);
			void Render(VkCommandBuffer commandBuffer, VkPipelineLayout _pipelineLayout);

		private:
			const Graphics::VulkanContext& m_VulkanContextRef;
			const Graphics::VulkanCommandPool& m_VulkanCommandPool;

			std::vector<Object> m_Points;
			std::vector<PointVertex> m_PointVertices;

			std::unique_ptr<Graphics::VulkanVertexBuffer<PointVertex>> m_VertexBuffer;

			void UpdateCurve();
	};
}