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
	}

	void Curve::UpdateCurve()
	{
		if (m_Points.size() < 4)
			return;
		m_PointVertices.clear();
		for (float t = 0.0f; t < 1.0f; t += 0.01f)
		{
			glm::vec3 curPoint = Math::CubicBezier(m_Points[0].position, m_Points[1].position, m_Points[2].position, m_Points[3].position, t);
			m_PointVertices.push_back({ curPoint, glm::vec3(t, 1.0f, 1.0f) });
		}

		m_VertexBuffer = std::make_unique<Graphics::VulkanVertexBuffer<PointVertex>>(m_VulkanContextRef, m_VulkanCommandPool, m_PointVertices.data(), m_PointVertices.size());
	}
}