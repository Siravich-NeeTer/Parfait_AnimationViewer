#pragma once

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <map>

#include "Core/Object.h"

#include "Math/AssimpGLMHelpers.h"

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

	struct BoneInfo
	{
		int id;
		glm::mat4 offset;
	};

	class Model : public Object
	{
		public:
			Model(const Graphics::VulkanContext& _vulkanContext, const Graphics::VulkanCommandPool& _vulkanCommandPool, const std::filesystem::path& _path);
			void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout _pipelineLayout);
			void DrawBone(VkCommandBuffer commandBuffer, VkPipelineLayout _pipelineLayout);

			std::map<std::string, BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }
			int& GetBoneCount() { return m_BoneCounter; }

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

			std::vector<Graphics::BoneVertex> m_BoneVertices;
			std::vector<uint32_t> m_BoneIndices;

			std::vector<Node*> m_Nodes;
			std::map<std::string, BoneInfo> m_BoneInfoMap;
			int m_BoneCounter = 0;

			std::string m_Directory;

			std::unique_ptr<Graphics::VulkanDescriptor> m_Descriptor;

			std::unique_ptr<Graphics::VulkanVertexBuffer<Graphics::Vertex>> m_VertexBuffer;
			std::unique_ptr<Graphics::VulkanIndexBuffer> m_IndexBuffer;
			std::vector<std::unique_ptr<Graphics::VulkanTexture>> m_Textures;

			std::unique_ptr<Graphics::VulkanVertexBuffer<Graphics::BoneVertex>> m_BoneVertexBuffer;
			std::unique_ptr<Graphics::VulkanIndexBuffer> m_BoneIndexBuffer;


			void LoadModel(const std::filesystem::path& _path);
			void ProcessNode(aiNode* _node, const aiScene* _scene, Node* _parent);
			void ProcessMesh(aiMesh* _mesh, const aiScene* _scene, Node* _currentNode);

			void DrawNode(VkCommandBuffer commandBuffer, Node* _node);

			void SetVertexBoneDataToDefault(Graphics::Vertex& _vertex);
			void SetVertexBoneData(Graphics::Vertex& _vertex, int _boneID, float _weight);
			void ExtractBoneWeightForVertices(std::vector<Graphics::Vertex>& _vertices, uint32_t _startIdx, aiMesh* _mesh, const aiScene* _scene);

			std::string GetDirectory(const std::filesystem::path& _path);
	};
}