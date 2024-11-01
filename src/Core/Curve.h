#pragma once

#include <vector>
#include <string>
#include <map>

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
			void Render(VkCommandBuffer commandBuffer, VkPipelineLayout _curvePipelineLayout);
			void RenderPoint(VkCommandBuffer commandBuffer, VkPipelineLayout _spherePointPipelineLayout);

			void UpdateCurve();

			glm::vec3 QueryPoint(float _t);

			std::vector<glm::vec3> GetPositionList() const
			{
				std::vector<glm::vec3> positions(m_PointVertices.size());
				for (int i = 0; i < m_PointVertices.size(); i++)
				{
					positions[i] = m_PointVertices[i].position;
				}
				return positions;
			}
			std::vector<Object>& GetPointObject() { return m_Points; }

		private:
			const Graphics::VulkanContext& m_VulkanContextRef;
			const Graphics::VulkanCommandPool& m_VulkanCommandPool;

			std::vector<Object> m_Points;
			std::vector<PointVertex> m_PointVertices;
			std::vector<glm::vec3> m_SphereVertices;

			// Build-Table
			std::vector<float> m_EventPoint_t;	// P0 = t0, Pn = tn
			std::vector<float> m_EventPoint_arclength;
			std::map<float, float> m_ArcLengthTable;
			std::map<float, glm::vec3> m_PointTable;

			std::unique_ptr<Graphics::VulkanVertexBuffer<PointVertex>> m_VertexBuffer;
			std::unique_ptr<Graphics::VulkanVertexBuffer<glm::vec3>> m_SpherePointBuffer;

			void BuildTable();
			void ClearTable();
			void ClearEventPoint();

	};
}