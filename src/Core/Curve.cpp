#include "Curve.h"

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

			for (float t = 0.0f; t <= 1.0f; t += 0.01f)
			{
				m_PointVertices.push_back({ Math::CatmullRom(P0, P1, P2, P3, t), glm::vec3(1.0f, 0.0f, 0.0f) });
			}
		}

		m_VertexBuffer = std::make_unique<Graphics::VulkanVertexBuffer<PointVertex>>(m_VulkanContextRef, m_VulkanCommandPool, m_PointVertices.data(), m_PointVertices.size());
	}
}