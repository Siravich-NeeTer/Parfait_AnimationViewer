#pragma once

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include "Renderer/Buffers/VulkanVertexBuffer.h"
#include "Renderer/Buffers/VulkanIndexBuffer.h"
#include "Renderer/Utilities/VulkanUtilities.h"

namespace Parfait
{
	namespace Graphics
	{
		class VulkanContext;
		class VulkanCommandPool;
	}

	/*
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	class Mesh
	{
		public:

		private:
			std::vector<Vertex> m_Vertices;
			std::vector<uint32_t> m_Indices;
	};
	*/

	class Model
	{
		public:
			Model(const Graphics::VulkanContext& _vulkanContext, const Graphics::VulkanCommandPool& _vulkanCommandPool, const std::filesystem::path& _path);
			void Draw(VkCommandBuffer commandBuffer);

		private:
			//std::vector<Mesh> m_Meshes;

			std::vector<Graphics::Vertex> m_Vertices;
			std::vector<uint32_t> m_Indices;

			std::string m_Directory;

			std::unique_ptr<Graphics::VulkanVertexBuffer<Graphics::Vertex>> m_VertexBuffer;
			std::unique_ptr<Graphics::VulkanIndexBuffer> m_IndexBuffer;


			void LoadModel(const std::filesystem::path& _path);
			void ProcessNode(aiNode* _node, const aiScene* _scene);
			void ProcessMesh(aiMesh* _mesh, const aiScene* _scene);

	};
}