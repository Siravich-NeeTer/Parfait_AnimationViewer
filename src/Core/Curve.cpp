#include "Curve.h"

#include <queue>

#include "Math/MathUtility.h"
#include "Renderer/Utilities/PrimitiveMesh.h"

namespace Parfait
{
	Curve::Curve(const Graphics::VulkanContext& _vulkanContext, const Graphics::VulkanCommandPool& _vulkanCommandPool)
		: m_VulkanContextRef(_vulkanContext), m_VulkanCommandPool(_vulkanCommandPool)
	{
		m_SphereVertices = Primitive::CreateSphere(0.025f, 10, 10);
		m_SpherePointBuffer = std::make_unique<Graphics::VulkanVertexBuffer<glm::vec3>>(m_VulkanContextRef, m_VulkanCommandPool, m_SphereVertices.data(), m_SphereVertices.size());
	}
	void Curve::AddPoint(const glm::vec3& _newPosition)
	{
		// TODO: Find Proper ID for each point
		uint32_t tmpID = 0;

		Object newObject(tmpID,"");
		newObject.position = _newPosition;
		m_Points.push_back(std::move(newObject));
		m_PointVertices.push_back({ _newPosition, glm::vec3(1.0f) });

		UpdateCurve();
	}
	void Curve::Render(VkCommandBuffer commandBuffer, VkPipelineLayout _curvePipelineLayout)
	{
		glm::mat4 model(1.0f);

		const VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer->GetBuffer(), offsets);
		vkCmdPushConstants(commandBuffer, _curvePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
		vkCmdDraw(commandBuffer, m_PointVertices.size(), 1, 0, 0);
	}
	void Curve::RenderPoint(VkCommandBuffer commandBuffer, VkPipelineLayout _spherePointPipelineLayout)
	{
		const VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_SpherePointBuffer->GetBuffer(), offsets);
		for (size_t i = 0; i < m_Points.size(); i++)
		{
			glm::mat4 model(1.0f);
			model = glm::translate(model, m_Points[i].position);

			vkCmdPushConstants(commandBuffer, _spherePointPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
			vkCmdDraw(commandBuffer, m_SphereVertices.size(), 1, 0, 0);
		}
	}

	void Curve::UpdateCurve()
	{
		if (m_Points.size() < 4)
			return;
		m_PointVertices.clear();
		ClearEventPoint();
		ClearTable();

		std::vector<float> arclength(m_Points.size());
		float totalArcLength = 0.0f;
		for (int i = 0; i < m_Points.size(); i++)
		{
			int i0 = i - 1 < 0 ? m_Points.size() - 1 : i - 1;
			int i1 = i;
			int i2 = i + 1 > m_Points.size() - 1 ? (i + 1) % m_Points.size() : i + 1;
			int i3 = i + 2 > m_Points.size() - 1 ? (i + 2) % m_Points.size() : i + 2;

			const glm::vec3& P0 = m_Points[i0].position;
			const glm::vec3& P1 = m_Points[i1].position;
			const glm::vec3& P2 = m_Points[i2].position;
			const glm::vec3& P3 = m_Points[i3].position;

			float step = 0.01f;
			for (float t = 0.0f; t <= 1.0f; t += step)
			{
				m_PointVertices.push_back({ Math::CatmullRom(P0, P1, P2, P3, t), glm::vec3(1.0f, 0.0f, 0.0f) });

				if (t > 0.0f)
				{
					arclength[i] += glm::distance(m_PointVertices[m_PointVertices.size() - 1].position,
						m_PointVertices[m_PointVertices.size() - 2].position);
				}
			}
			totalArcLength += arclength[i];
		}

		float currentArcLength = 0.0f;
		m_EventPoint_t.push_back(currentArcLength);
		m_EventPoint_arclength.push_back(0.0f);
		for (int i = 0; i < m_Points.size(); i++)
		{
			currentArcLength += arclength[i];
			m_EventPoint_t.push_back(currentArcLength / totalArcLength);
			m_EventPoint_arclength.push_back(currentArcLength);
		}

		/*
		std::cout << "Curve ArcLength\n";
		for (int i = 0; i < m_EventPoint_t.size(); i++)
		{
			std::cout << m_EventPoint_t[i] << " : " << m_EventPoint_arclength[i] << "\n";
		}
		*/
		BuildTable();

		m_VertexBuffer = std::make_unique<Graphics::VulkanVertexBuffer<PointVertex>>(m_VulkanContextRef, m_VulkanCommandPool, m_PointVertices.data(), m_PointVertices.size());
	}

	glm::vec3 Curve::QueryPoint(float _t)
	{
		float queryArcLength = 0.0f;

		// Binary Search for querying the event point (t)
		auto it = std::lower_bound(m_EventPoint_t.begin(), m_EventPoint_t.end(), _t);

		int qIndex = (it == m_EventPoint_t.begin() ? 0 : std::distance(m_EventPoint_t.begin(), it) - 1);
		_t -= m_EventPoint_t[qIndex];

		int i0 = qIndex - 1 < 0 ? m_Points.size() - 1 : qIndex - 1;
		int i1 = qIndex;
		int i2 = qIndex + 1 > m_Points.size() - 1 ? (qIndex + 1) % m_Points.size() : qIndex + 1;
		int i3 = qIndex + 2 > m_Points.size() - 1 ? (qIndex + 2) % m_Points.size() : qIndex + 2;

		const glm::vec3& P0 = m_Points[i0].position;
		const glm::vec3& P1 = m_Points[i1].position;
		const glm::vec3& P2 = m_Points[i2].position;
		const glm::vec3& P3 = m_Points[i3].position;

		return Math::CatmullRom(P0, P1, P2, P3, _t / (m_EventPoint_t[qIndex + 1] - m_EventPoint_t[qIndex]));
	}

	void Curve::BuildTable()
	{
		// Arc-Length Error Threshold
		const float epsilon = 0.01f;
		// Maximum parameter interval
		const float delta = 0.01f;

		std::queue<std::pair<float, float>> segmentList;
		m_ArcLengthTable[0.0f] = 0.0f;
		m_PointTable[0.0f] = m_Points[0].position;
		segmentList.push({ 0.0f, 1.0f });
		while (!segmentList.empty())
		{
			float ua = segmentList.front().first;
			float ub = segmentList.front().second;
			float um = (ua + ub) / 2.0f;
			segmentList.pop();

			glm::vec3 P_ua = QueryPoint(ua);
			glm::vec3 P_ub = QueryPoint(ub);
			glm::vec3 P_um = QueryPoint(um);

			float A = glm::distance(P_ua, P_um);
			float B = glm::distance(P_um, P_ub);
			float C = glm::distance(P_ua, P_ub);

			float d = A + B - C;

			if (d > epsilon || std::fabs(ua - ub) > delta)
			{
				segmentList.push({ ua, um });
				segmentList.push({ um, ub});
			}
			else
			{
				m_ArcLengthTable[um] = m_ArcLengthTable[ua] + A;
				m_ArcLengthTable[ub] = m_ArcLengthTable[um] + B;

				m_PointTable[um] = P_um;
				m_PointTable[ub] = P_ub;
			}
		}
		return;
	}
	void Curve::ClearTable()
	{
		m_ArcLengthTable.clear();
		m_PointTable.clear();
	}
	void Curve::ClearEventPoint()
	{
		m_EventPoint_t.clear();
		m_EventPoint_arclength.clear();
	}
}