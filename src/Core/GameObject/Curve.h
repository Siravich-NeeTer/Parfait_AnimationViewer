#pragma once

#include <vector>
#include <string>
#include <map>

#include "Renderer/Buffers/VulkanVertexBuffer.h"

#include "Object.h"

namespace Parfait
{
	struct PointVertex
	{
		glm::vec3 position;
		glm::vec3 color;
	};

	class Curve : public Object
	{
		public:
			Curve(const Graphics::VulkanContext& _vulkanContext, const Graphics::VulkanCommandPool& _vulkanCommandPool);

			void AddPoint(const glm::vec3& _newPosition);
			void Render(VkCommandBuffer commandBuffer, VkPipelineLayout _curvePipelineLayout);
			void RenderPoint(VkCommandBuffer commandBuffer, VkPipelineLayout _spherePointPipelineLayout);

			// ImGui Related Functions
			void AddVelocity(float _t, float _velocity);
			void DisplayGraph();

			void UpdateCurve();
			
			float GetDistanceParameter(float _t) const
			{
				_t = ClampParameter(_t);

				const size_t distanceStepSize = m_DistanceStep.size() - 1;
				size_t prevIndex = _t * distanceStepSize;

				float t = m_DistanceStep[prevIndex];

				return t;
			}
			float GetVelocity(float _t) const
			{
				_t = ClampParameter(_t);

				const size_t distanceStepSize = m_VelocityStep.size() - 1;
				size_t prevIndex = _t * distanceStepSize;

				float velocity = m_VelocityStep[prevIndex];

				return velocity;
			}
			glm::vec3 GetPointFromTable(float _t) const
			{
				auto it = m_PointTable.lower_bound(_t);

				float next_t = (_t == 0.0f ? 0.0f : it->first);
				float prev_t = (_t == 0.0f ? 1.0f : (--it)->first);

				glm::vec3 nextPoint = m_PointTable.find(next_t)->second;
				glm::vec3 prevPoint = m_PointTable.find(prev_t)->second;

				return glm::mix(prevPoint, nextPoint, (_t - prev_t) / (next_t - prev_t));
			}
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

			#define GRAPH_SIZE 1001
			std::map<float, float> m_VelocityTable;
			// ImGui Related members
			std::array<float, GRAPH_SIZE> m_TStep;
			std::array<float, GRAPH_SIZE> m_VelocityStep;
			std::array<float, GRAPH_SIZE> m_DistanceStep;
			float m_SelectedEventTime;
			float m_SelectedEventVelocity;

			std::unique_ptr<Graphics::VulkanVertexBuffer<PointVertex>> m_VertexBuffer;
			std::unique_ptr<Graphics::VulkanVertexBuffer<glm::vec3>> m_SpherePointBuffer;
			
			void InitDisplayVelocity();
			void UpdateVelocityTable(float _eventT);
			void UpdateDistanceTable();

			glm::vec3 QueryPoint(float _t);
			void BuildTable();
			void ClearTable();
			void ClearEventPoint();

			// Helper Functions
			float ClampParameter(float _t) const
			{
				if (_t < 0.0f)
					_t = std::fabs(_t);
				if (_t > 1.0f)
					_t -= (int)_t;
				return _t;
			}

	};
}