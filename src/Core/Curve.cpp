#include "Curve.h"

#include "Math/MathUtility.h"

namespace Parfait
{
	Curve::Curve(const Graphics::VulkanContext& _vulkanContext, const Graphics::VulkanCommandPool& _vulkanCommandPool)
		: m_VulkanContextRef(_vulkanContext), m_VulkanCommandPool(_vulkanCommandPool)
	{

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
	void Curve::Render(VkCommandBuffer commandBuffer, VkPipelineLayout _pipelineLayout)
	{
		glm::mat4 model(1.0f);

		const VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer->GetBuffer(), offsets);
		vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
		vkCmdDraw(commandBuffer, m_PointVertices.size(), 1, 0, 0);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_TempVertexBuffer->GetBuffer(), offsets);
		vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
		vkCmdDraw(commandBuffer, m_TempPointVertices.size(), 1, 0, 0);
	}

	void Curve::UpdateCurve()
	{
		if (m_Points.size() < 4)
			return;
		else if (m_Points.size() % 3 == 2)
		{
			size_t curIdx = m_Points.size() - 1;
			glm::vec3 newDir = glm::normalize(m_Points[curIdx - 1].position - m_Points[curIdx - 2].position);
			m_Points[curIdx].position = m_Points[curIdx - 1].position + newDir * glm::distance(m_Points[curIdx].position, m_Points[curIdx - 1].position);
		}
		m_PointVertices.clear();
		m_TempPointVertices.clear();

		std::vector<glm::vec3> controlPoints;
		for (int i = 0; i < m_Points.size(); i++)
		{
			controlPoints.push_back(m_Points[i].position);
		}

		for (float t = 0.0f; t < 1.0f; t += 0.001f)
		{
			glm::vec3 curPoint = Math::ComputeBezier(controlPoints, t);
			m_PointVertices.push_back({ curPoint, glm::vec3(t, 1.0f, 1.0f) });
		}

		for (int i = 0; i < m_Points.size(); i++)
		{
			m_TempPointVertices.push_back({ m_Points[i].position, i % 3 == 0 ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(1.0f)});
		}

		m_VertexBuffer = std::make_unique<Graphics::VulkanVertexBuffer<PointVertex>>(m_VulkanContextRef, m_VulkanCommandPool, m_PointVertices.data(), m_PointVertices.size());
		m_TempVertexBuffer = std::make_unique<Graphics::VulkanVertexBuffer<PointVertex>>(m_VulkanContextRef, m_VulkanCommandPool, m_TempPointVertices.data(), m_TempPointVertices.size());
	}
}