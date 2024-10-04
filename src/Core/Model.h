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

#include "Renderer/VulkanDescriptor.h"
#include "Renderer/VulkanTexture.h"

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

	class Model
	{
		public:
			Model(const Graphics::VulkanContext& _vulkanContext, const Graphics::VulkanCommandPool& _vulkanCommandPool, const std::filesystem::path& _path);
			void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout _pipelineLayout);

			const Graphics::VulkanDescriptor& GetDescriptor() const { return *m_Descriptor; }

		private:
			const Graphics::VulkanContext& m_VulkanContextRef;
			const Graphics::VulkanCommandPool& m_VulkanCommandPool;
			VkPipelineLayout m_PipelineLayoutRef;

			struct Primitive
			{
				uint32_t firstIndex;
				uint32_t indexCount;
				int32_t materialIndex;
			};
			struct Mesh
			{
				std::vector<Primitive> primitives;
			};
			struct Node
			{
				Node* parent;
				std::vector<Node*> children;
				Mesh mesh;
				glm::mat4 matrix;

				~Node()
				{
					for (Node* child : children)
					{
						delete child;
					}
				}
			};

			std::vector<Graphics::Vertex> m_Vertices;
			std::vector<uint32_t> m_Indices;
			std::vector<Node*> m_Nodes;

			std::string m_Directory;

			std::unique_ptr<Graphics::VulkanDescriptor> m_Descriptor;

			std::unique_ptr<Graphics::VulkanVertexBuffer<Graphics::Vertex>> m_VertexBuffer;
			std::unique_ptr<Graphics::VulkanIndexBuffer> m_IndexBuffer;
			std::vector<std::unique_ptr<Graphics::VulkanTexture>> m_Textures;


			void LoadModel(const std::filesystem::path& _path);
			void ProcessNode(aiNode* _node, const aiScene* _scene, Node* _parent);
			void ProcessMesh(aiMesh* _mesh, const aiScene* _scene, Node* _currentNode);

			void DrawNode(VkCommandBuffer commandBuffer, Node* _node);
	};
}